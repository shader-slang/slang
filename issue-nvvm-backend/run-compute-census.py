#!/usr/bin/env python3

"""Run the repository CUDA compute corpus through native CUDA and direct NVVM.

The script never edits `tests/`. It copies the test tree below `build/nvvm-census`, removes
execution directives from the mirror, and creates one generated sibling source per active native
CUDA COMPARE_COMPUTE directive. Keeping one directive per generated source gives every workload a
stable identity and an independently timed log while preserving its source and TEST_INPUT metadata.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import csv
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import threading
import time


ACTIVE_CUDA_RE = re.compile(
    r"^(?P<indent>\s*)//TEST(?P<categories>\([^)]*\))?:"
    r"(?P<command>COMPARE_COMPUTE(?:_EX)?(?:\([^)]*\))?):(?P<arguments>.*-cuda.*)$",
    re.IGNORECASE,
)
EXECUTION_DIRECTIVE_RE = re.compile(
    r"^\s*//(?:TEST(?:\([^)]*\))?|DISABLED?_TEST(?:\([^)]*\))?):",
    re.IGNORECASE,
)
ACTIVE_EXECUTION_DIRECTIVE_RE = re.compile(
    r"^\s*//TEST(?:\([^)]*\))?:",
    re.IGNORECASE,
)
OPTIMIZATION_RE = re.compile(r"\s+(?:-Xslang|-xslang)\s+-O[0-3]\b", re.IGNORECASE)
DIRECT_NVVM_RE = re.compile(
    r"\s+(?:-Xslang|-xslang)\s+-emit-cuda-via-nvvm\b",
    re.IGNORECASE,
)
CAPABILITY_RE = re.compile(r"(?:^|\s)-capability\s+(?P<capability>\S+)", re.IGNORECASE)
DIAGNOSTIC_RE = re.compile(r"error\[(E\d+)\]:\s*([^\r\n]+)", re.IGNORECASE)
PREFLIGHT_SHAPE_RE = re.compile(
    r"direct NVVM lowering does not support Slang IR instruction or shape '([^']+)'",
    re.IGNORECASE,
)


EXCLUDED_PREFIXES = {
    "autodiff/": "automatic differentiation is outside the initial compute MVP",
    "autodiff-dstdlib/": "automatic differentiation is outside the initial compute MVP",
    "cooperative-matrix/": "cooperative matrices are outside the initial compute MVP",
    "neural/": "neural/cooperative-matrix workloads are outside the initial compute MVP",
    "pipeline/ray-tracing/": "ray tracing and OptiX-style pipelines are outside the initial MVP",
}


EXCLUDED_PATH_PARTS = {
    "fp8": "FP8 is outside the initial compute MVP",
    "dynamic-parallel": "CUDA dynamic parallelism is outside the initial compute MVP",
    "device-lto": "RDC/device LTO is outside the initial compute MVP",
    "device-syscall": "device syscalls are outside the initial compute MVP",
}


EXTENSION_WAVE_RE = re.compile(
    r"^(?:hlsl-intrinsic/wave-rotate/|hlsl-intrinsic/wave-multi/|"
    r"hlsl-intrinsic/wave-prefix-|hlsl-intrinsic/wave-mask/wave-(?:mask-)?prefix-|"
    r"hlsl-intrinsic/quad-control/)|(?:^|/)wave-matrix\.slang$",
    re.IGNORECASE,
)


MODES = {
    "nvrtc-o3": (False, 3),
    "nvvm-o0": (True, 0),
    "nvvm-o3": (True, 3),
}


RESULT_FIELDS = [
    "id",
    "source",
    "source_line",
    "source_test_ordinal",
    "cuda_ordinal",
    "mode",
    "capability",
    "reference_derived_from_direct",
    "classification",
    "return_code",
    "elapsed_ms",
    "diagnostic",
    "canonical_shape",
    "log",
]


def _normalize_relative(path: Path) -> str:
    return path.as_posix()


def _read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8-sig", errors="strict")
    except UnicodeDecodeError:
        return path.read_text(encoding="cp1252", errors="strict")


def _write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8", newline="\n")


def _require_mode_root(output_root: Path, mode_root: Path, mode: str) -> Path:
    expected = (output_root / "mirrors" / mode).resolve()
    actual = mode_root.resolve()
    if mode not in MODES or actual != expected or actual == output_root.resolve():
        raise SystemExit(f"refusing to replace unexpected census mirror path: {actual}")
    return actual


def _exclusion_reason(relative_path: str) -> str | None:
    lowered = relative_path.lower()
    for prefix, reason in EXCLUDED_PREFIXES.items():
        if lowered.startswith(prefix):
            return reason
    for part, reason in EXCLUDED_PATH_PARTS.items():
        if part in lowered:
            return reason
    return None


def _coverage_tier(relative_path: str) -> tuple[str, str]:
    if EXTENSION_WAVE_RE.search(relative_path):
        return "extension", "advanced wave/quad operation outside the initial MVP"
    if relative_path.lower() == "slang-extension/realtime-clock.slang":
        return "extension", "device clock/syscall operation outside the initial MVP"
    return "mvp", ""


def discover_workloads(tests_dir: Path) -> tuple[list[dict[str, object]], list[dict[str, str]]]:
    workloads: list[dict[str, object]] = []
    excluded: list[dict[str, str]] = []
    for source_path in sorted(tests_dir.rglob("*.slang")):
        if not source_path.is_file():
            continue
        relative_path = _normalize_relative(source_path.relative_to(tests_dir))
        native_directives: list[dict[str, object]] = []
        direct_directives: list[dict[str, object]] = []
        test_ordinal = 0
        for line_number, line in enumerate(_read_text(source_path).splitlines(), start=1):
            match = ACTIVE_CUDA_RE.match(line)
            if match:
                directive = {
                    "line": line_number,
                    "test_ordinal": test_ordinal,
                    "categories": match.group("categories") or "",
                    "command": match.group("command"),
                    "arguments": match.group("arguments").strip(),
                    "reference_derived_from_direct": False,
                }
                if "emit-cuda-via-nvvm" in match.group("arguments").lower():
                    directive["reference_derived_from_direct"] = True
                    direct_directives.append(directive)
                else:
                    native_directives.append(directive)
            if ACTIVE_EXECUTION_DIRECTIVE_RE.match(line):
                test_ordinal += 1

        if not native_directives and direct_directives:
            native_directives = direct_directives
        if not native_directives:
            continue

        reason = _exclusion_reason(relative_path)
        if reason:
            excluded.append({"source": relative_path, "reason": reason})
            continue

        for ordinal, directive in enumerate(native_directives, start=1):
            workload_id = f"{relative_path}#cuda-{ordinal}"
            capability_match = CAPABILITY_RE.search(str(directive["arguments"]))
            coverage_tier, scope_reason = _coverage_tier(relative_path)
            workloads.append(
                {
                    "id": workload_id,
                    "source": relative_path,
                    "source_line": directive["line"],
                    "source_test_ordinal": directive["test_ordinal"],
                    "cuda_ordinal": ordinal,
                    "categories": directive["categories"],
                    "command": directive["command"],
                    "arguments": directive["arguments"],
                    "reference_derived_from_direct": directive["reference_derived_from_direct"],
                    "coverage_tier": coverage_tier,
                    "scope_reason": scope_reason,
                    "capability": capability_match.group("capability")
                    if capability_match
                    else "cuda_sm_7_0",
                }
            )
    return workloads, excluded


def _directive_for_mode(workload: dict[str, object], mode: str) -> str:
    use_nvvm, optimization = MODES[mode]
    arguments = OPTIMIZATION_RE.sub("", str(workload["arguments"])).strip()
    arguments = DIRECT_NVVM_RE.sub("", arguments).strip()
    if not CAPABILITY_RE.search(arguments):
        arguments += " -capability cuda_sm_7_0"
    if use_nvvm:
        arguments += " -Xslang -emit-cuda-via-nvvm"
    arguments += f" -Xslang -O{optimization}"
    return (
        f"//TEST{workload['categories']}:{workload['command']}:"
        f"{arguments}\n"
    )


def _generated_relative_path(workload: dict[str, object]) -> Path:
    source = Path(str(workload["source"]))
    digest = hashlib.sha1(str(workload["id"]).encode("utf-8")).hexdigest()[:10]
    return source.parent / f"nvvm-census-{source.stem}-{digest}.slang"


def prepare_mode(
    tests_dir: Path,
    output_root: Path,
    mode_root: Path,
    workloads: list[dict[str, object]],
    mode: str,
) -> None:
    mode_root = _require_mode_root(output_root, mode_root, mode)
    if mode_root.exists():
        shutil.rmtree(mode_root)
    shutil.copytree(tests_dir, mode_root)

    for workload in workloads:
        original_source = tests_dir / Path(str(workload["source"]))
        lines = _read_text(original_source).splitlines(keepends=True)
        filtered = [line for line in lines if not EXECUTION_DIRECTIVE_RE.match(line)]
        generated_path = mode_root / _generated_relative_path(workload)
        _write_text(generated_path, _directive_for_mode(workload, mode) + "".join(filtered))
        source_test_ordinal = int(workload["source_test_ordinal"])
        expected_suffix = (
            ".expected.txt"
            if source_test_ordinal == 0
            else f".{source_test_ordinal}.expected.txt"
        )
        expected_source = Path(str(original_source) + expected_suffix)
        if not expected_source.is_file() and source_test_ordinal:
            expected_source = Path(str(original_source) + ".expected.txt")
        if expected_source.is_file():
            shutil.copy2(expected_source, Path(str(generated_path) + ".expected.txt"))


def _classify_result(return_code: int, output: str, mode: str) -> tuple[str, str, str]:
    diagnostic_match = DIAGNOSTIC_RE.search(output)
    diagnostic_code = diagnostic_match.group(1).upper() if diagnostic_match else ""
    diagnostic = diagnostic_match.group(2).strip() if diagnostic_match else ""
    shape_match = PREFLIGHT_SHAPE_RE.search(output)
    shape = shape_match.group(1) if shape_match else ""
    for typed_role in (
        "helper function parameter",
        "helper function result type",
        "call argument type",
        "immutable struct field access",
        "sequential element pointer",
    ):
        if shape.startswith(f"{typed_role}:"):
            shape = typed_role
            break

    if "no tests run" in output.lower():
        return "infrastructure", "generated census test was not discovered", shape
    if return_code == 0:
        return "correct", diagnostic, shape
    if diagnostic_code == "E52017":
        return "preflight", diagnostic, shape
    if diagnostic_code == "E52018" or "NVVM IR verification" in output or "libNVVM" in output:
        return "provider", diagnostic, shape
    infrastructure_markers = (
        "E52016",
        "unable to load a compatible LLVM",
        "no CUDA device",
        "CUDA driver version is insufficient",
        "Unsupported backend",
        "Check cuda: Not Supported",
    )
    if any(marker.lower() in output.lower() for marker in infrastructure_markers):
        return "infrastructure", diagnostic, shape
    if mode.startswith("nvrtc-") and (
        re.search(r"nvrtc[^\r\n]*:\s*(?:error|.* error )", output, re.IGNORECASE)
        or "profile implicitly upgraded" in output
    ):
        return "infrastructure", diagnostic, shape
    if "EXPECTED{{{" in output and "ACTUAL{{{" in output:
        return "runtime-mismatch", diagnostic, shape
    if re.search(r"slang-test:.*: error: [A-Z][A-Z0-9_-]*", output, re.IGNORECASE):
        return "runtime-mismatch", diagnostic, shape
    if mode.startswith("nvrtc-"):
        return "infrastructure", diagnostic, shape
    return "unclassified", diagnostic, shape


def _run_one(
    repo_root: Path,
    output_root: Path,
    mode_root: Path,
    workload: dict[str, object],
    mode: str,
    provider_path: Path,
) -> dict[str, object]:
    generated_relative = _generated_relative_path(workload)
    command = [
        str(repo_root / "build/Release/bin/slang-test.exe"),
        "-test-dir",
        str(mode_root),
        "-disable-retries",
        _normalize_relative((mode_root / generated_relative).relative_to(repo_root)),
    ]
    environment = os.environ.copy()
    environment["SLANG_NVVM_BUILDER_PATH"] = str(provider_path)
    started = time.perf_counter()
    completed = subprocess.run(
        command,
        cwd=repo_root,
        env=environment,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )
    elapsed_ms = round((time.perf_counter() - started) * 1000)
    classification, diagnostic, shape = _classify_result(
        completed.returncode,
        completed.stdout,
        mode,
    )
    log_name = hashlib.sha1(f"{mode}:{workload['id']}".encode("utf-8")).hexdigest() + ".log"
    log_path = output_root / "logs" / mode / log_name
    _write_text(log_path, completed.stdout)
    return {
        "id": workload["id"],
        "source": workload["source"],
        "source_line": workload["source_line"],
        "source_test_ordinal": workload["source_test_ordinal"],
        "cuda_ordinal": workload["cuda_ordinal"],
        "mode": mode,
        "capability": workload["capability"],
        "reference_derived_from_direct": workload["reference_derived_from_direct"],
        "classification": classification,
        "return_code": completed.returncode,
        "elapsed_ms": elapsed_ms,
        "diagnostic": diagnostic,
        "canonical_shape": shape,
        "log": _normalize_relative(log_path.relative_to(repo_root)),
    }


def run_mode(
    repo_root: Path,
    output_root: Path,
    mode_root: Path,
    workloads: list[dict[str, object]],
    mode: str,
    provider_path: Path,
    jobs: int,
) -> list[dict[str, object]]:
    results: list[dict[str, object]] = []
    print(f"running {mode}: {len(workloads)} workloads with {jobs} workers", flush=True)
    started = time.perf_counter()
    print_lock = threading.Lock()
    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as executor:
        futures = {
            executor.submit(
                _run_one,
                repo_root,
                output_root,
                mode_root,
                workload,
                mode,
                provider_path,
            ): workload
            for workload in workloads
        }
        for future in concurrent.futures.as_completed(futures):
            result = future.result()
            results.append(result)
            with print_lock:
                if len(results) % 20 == 0 or len(results) == len(workloads):
                    elapsed = time.perf_counter() - started
                    print(
                        f"{mode}: {len(results)}/{len(workloads)} complete in {elapsed:.1f}s",
                        flush=True,
                    )
    return sorted(results, key=lambda result: str(result["id"]))


def _write_tsv(path: Path, rows: list[dict[str, object]], fields: list[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields, delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def _write_result_files(
    output_root: Path,
    results: list[dict[str, object]],
) -> dict[str, dict[str, int]]:
    _write_tsv(output_root / "results.tsv", results, RESULT_FIELDS)
    _write_text(output_root / "results.json", json.dumps(results, indent=2) + "\n")
    counts: dict[str, dict[str, int]] = {}
    for result in results:
        mode_counts = counts.setdefault(str(result["mode"]), {})
        classification = str(result["classification"])
        mode_counts[classification] = mode_counts.get(classification, 0) + 1
    _write_text(output_root / "summary.json", json.dumps(counts, indent=2) + "\n")
    return counts


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--output", type=Path, default=Path("build/nvvm-census"))
    parser.add_argument("--jobs", type=int, default=8)
    parser.add_argument("--discover-only", action="store_true")
    parser.add_argument(
        "--classify-only",
        action="store_true",
        help="Reclassify the preserved results.json logs without rerunning tests.",
    )
    parser.add_argument("--keep-mirrors", action="store_true")
    parser.add_argument(
        "--match",
        help=(
            "Run only workload IDs containing this case-insensitive substring "
            "(for runner probes)."
        ),
    )
    parser.add_argument(
        "--match-regex",
        help="Run only workload IDs matching this case-insensitive regular expression.",
    )
    parser.add_argument(
        "--modes",
        nargs="+",
        choices=sorted(MODES),
        default=list(MODES),
    )
    return parser.parse_args()


def main() -> int:
    args = parse_arguments()
    repo_root = args.repo.resolve()
    output_root = (
        (repo_root / args.output).resolve() if not args.output.is_absolute() else args.output
    )
    tests_dir = repo_root / "tests"
    provider_path = repo_root / "build/nvvm-builder-deps/slang-llvm-nvvm-build/Release"
    test_runner = repo_root / "build/Release/bin/slang-test.exe"
    if not test_runner.is_file():
        raise SystemExit(f"missing Release test runner: {test_runner}")
    if not provider_path.is_dir():
        raise SystemExit(f"missing NVVM provider directory: {provider_path}")

    workloads, excluded = discover_workloads(tests_dir)
    candidate_sources = {str(workload["source"]) for workload in workloads}
    candidate_sources.update(item["source"] for item in excluded)
    manifest = {
        "schema": 1,
        "candidate_source_count": len(candidate_sources),
        "eligible_source_count": len({str(workload["source"]) for workload in workloads}),
        "eligible_workload_count": len(workloads),
        "mvp_workload_count": sum(
            1 for workload in workloads if workload["coverage_tier"] == "mvp"
        ),
        "extension_workload_count": sum(
            1 for workload in workloads if workload["coverage_tier"] == "extension"
        ),
        "excluded_source_count": len(excluded),
        "modes": args.modes,
        "workloads": workloads,
        "excluded": excluded,
    }
    output_root.mkdir(parents=True, exist_ok=True)
    _write_text(output_root / "manifest.json", json.dumps(manifest, indent=2) + "\n")
    _write_tsv(
        output_root / "eligible-workloads.tsv",
        workloads,
        [
            "id",
            "source",
            "source_line",
            "source_test_ordinal",
            "cuda_ordinal",
            "capability",
            "reference_derived_from_direct",
            "coverage_tier",
            "scope_reason",
            "categories",
            "command",
            "arguments",
        ],
    )
    _write_tsv(output_root / "excluded-sources.tsv", excluded, ["source", "reason"])
    print(
        "discovered "
        f"{manifest['candidate_source_count']} candidate sources, "
        f"{manifest['eligible_source_count']} eligible sources, and "
        f"{manifest['eligible_workload_count']} eligible CUDA workloads",
        flush=True,
    )
    if args.discover_only:
        return 0
    if args.classify_only:
        results_path = output_root / "results.json"
        if not results_path.is_file():
            raise SystemExit(f"missing prior census results: {results_path}")
        results = json.loads(_read_text(results_path))
        for result in results:
            log_path = repo_root / Path(str(result["log"]))
            classification, diagnostic, shape = _classify_result(
                int(result["return_code"]),
                _read_text(log_path),
                str(result["mode"]),
            )
            result["classification"] = classification
            result["diagnostic"] = diagnostic
            result["canonical_shape"] = shape
        counts = _write_result_files(output_root, results)
        print(json.dumps(counts, indent=2), flush=True)
        return 0 if all("unclassified" not in item for item in counts.values()) else 2

    run_workloads = workloads
    if args.match and args.match_regex:
        raise SystemExit("--match and --match-regex are mutually exclusive")
    if args.match:
        needle = args.match.lower()
        run_workloads = [
            workload for workload in workloads if needle in str(workload["id"]).lower()
        ]
        if not run_workloads:
            raise SystemExit(f"no eligible workload ID contains: {args.match}")
        print(f"selected {len(run_workloads)} workloads matching {args.match!r}", flush=True)
    elif args.match_regex:
        try:
            selection = re.compile(args.match_regex, re.IGNORECASE)
        except re.error as error:
            raise SystemExit(f"invalid --match-regex: {error}") from error
        run_workloads = [
            workload for workload in workloads if selection.search(str(workload["id"]))
        ]
        if not run_workloads:
            raise SystemExit(f"no eligible workload ID matches: {args.match_regex}")
        print(
            f"selected {len(run_workloads)} workloads matching the regular expression",
            flush=True,
        )

    all_results: list[dict[str, object]] = []
    for mode in args.modes:
        mode_root = output_root / "mirrors" / mode
        print(f"preparing generated {mode} mirror", flush=True)
        prepare_mode(tests_dir, output_root, mode_root, run_workloads, mode)
        all_results.extend(
            run_mode(
                repo_root,
                output_root,
                mode_root,
                run_workloads,
                mode,
                provider_path,
                args.jobs,
            )
        )
        if not args.keep_mirrors:
            shutil.rmtree(_require_mode_root(output_root, mode_root, mode))

    counts = _write_result_files(output_root, all_results)
    print(json.dumps(counts, indent=2), flush=True)
    return 0 if all("unclassified" not in mode_counts for mode_counts in counts.values()) else 2


if __name__ == "__main__":
    sys.exit(main())
