#!/usr/bin/env python3

"""Run the rolling compute discovery corpus without changing frozen corpus v1.

The checked-in manifest selects one existing active compare-compute contract from each source by
its source-test ordinal. This script verifies that no selected source is represented in frozen
corpus v1, adapts only target-specific harness arguments to native CUDA in a disposable mirror,
and reuses the established census machinery for NVRTC O3 and direct NVVM O0/O3 execution.
"""

from __future__ import annotations

import argparse
import csv
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import sys
from types import ModuleType


COMPARE_DIRECTIVE_RE = re.compile(
    r"^(?P<indent>\s*)//TEST(?P<categories>\([^)]*\))?:"
    r"(?P<command>COMPARE_COMPUTE(?:_EX)?(?:\([^)]*\))?):(?P<arguments>.*)$",
    re.IGNORECASE,
)
ACTIVE_EXECUTION_DIRECTIVE_RE = re.compile(
    r"^\s*//TEST(?:\([^)]*\))?:",
    re.IGNORECASE,
)
ARGUMENT_TOKEN_RE = re.compile(r'''"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*'|\S+''')


TARGET_FLAGS = {
    "-cpu",
    "-d3d11",
    "-d3d12",
    "-dx11",
    "-dx12",
    "-glsl",
    "-hlsl",
    "-llvm",
    "-metal",
    "-mtl",
    "-vk",
    "-vulkan",
    "-wgpu",
    "-wgsl",
}


TARGET_EMISSION_FLAGS = {
    "-emit-spirv-directly",
    "-emit-spirv-via-glsl",
    "-use-dxbc",
}


TARGET_OPTIONS_WITH_VALUE = {
    "-capability",
    "-profile",
    "-target",
}


EXPECTED_V1 = {
    "rows": 452,
    "sources": 448,
    "healthy_mvp": 427,
    "o0_correct": 371,
    "o3_correct": 375,
    "both_correct": 371,
}


REQUIRED_SELECTION_TAGS = {
    "aggregate-pointer",
    "atomic-wave",
    "control-flow",
    "helper-generic",
    "large",
    "matrix-layout",
    "mixed-resources",
    "parameter-layout",
    "shared-barrier",
}


UNAVAILABLE_ENTRY_POINT_RE = re.compile(
    r"error\[E36107\]:\s*unavailable features in entry point",
    re.IGNORECASE,
)


def _load_census_module(script_dir: Path) -> ModuleType:
    module_path = script_dir / "run-compute-census.py"
    spec = importlib.util.spec_from_file_location("nvvm_compute_census", module_path)
    if spec is None or spec.loader is None:
        raise SystemExit(f"unable to load census helper module: {module_path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _install_discovery_classifier(census: ModuleType) -> None:
    base_classifier = census._classify_result

    def classify(return_code: int, output: str, mode: str) -> tuple[str, str, str]:
        if UNAVAILABLE_ENTRY_POINT_RE.search(output):
            return (
                "infrastructure",
                "E36107: target capability requirements make the entry point unavailable to CUDA",
                "target capability requirement",
            )
        return base_classifier(return_code, output, mode)

    census._classify_result = classify


def _resolve_path(repo_root: Path, path: Path) -> Path:
    return path.resolve() if path.is_absolute() else (repo_root / path).resolve()


def _read_tsv(path: Path) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8", newline="") as stream:
        return list(csv.DictReader(stream, delimiter="\t"))


def _audit_frozen_v1(path: Path) -> tuple[set[str], dict[str, int]]:
    rows = _read_tsv(path)
    sources = {row["source"].lower() for row in rows}
    healthy_mvp = [
        row
        for row in rows
        if row["coverage_tier"] == "mvp" and row["nvrtc-o3_classification"] == "correct"
    ]
    observed = {
        "rows": len(rows),
        "sources": len(sources),
        "healthy_mvp": len(healthy_mvp),
        "o0_correct": sum(
            row["nvvm-o0_classification"] == "correct" for row in healthy_mvp
        ),
        "o3_correct": sum(
            row["nvvm-o3_classification"] == "correct" for row in healthy_mvp
        ),
        "both_correct": sum(
            row["nvvm-o0_classification"] == "correct"
            and row["nvvm-o3_classification"] == "correct"
            for row in healthy_mvp
        ),
    }
    if observed != EXPECTED_V1:
        raise SystemExit(
            "frozen corpus-v1 contract changed:\n"
            f"expected {json.dumps(EXPECTED_V1, sort_keys=True)}\n"
            f"observed {json.dumps(observed, sort_keys=True)}"
        )
    return sources, observed


def _find_compare_directive(
    source_text: str,
    source_test_ordinal: int,
) -> dict[str, object]:
    current_ordinal = 0
    for line_number, line in enumerate(source_text.splitlines(), start=1):
        match = COMPARE_DIRECTIVE_RE.match(line)
        if match and current_ordinal == source_test_ordinal:
            return {
                "line": line_number,
                "categories": match.group("categories") or "",
                "command": match.group("command"),
                "arguments": match.group("arguments").strip(),
            }
        if ACTIVE_EXECUTION_DIRECTIVE_RE.match(line):
            current_ordinal += 1
    raise ValueError(
        f"no active compare-compute directive at source-test ordinal {source_test_ordinal}"
    )


def _adapt_arguments_to_cuda(arguments: str) -> str:
    tokens = ARGUMENT_TOKEN_RE.findall(arguments)
    adapted: list[str] = []
    index = 0
    while index < len(tokens):
        token = tokens[index]
        lowered = token.lower()
        if lowered == "-cuda":
            raise ValueError("discovery source already has a native CUDA directive")
        if lowered in TARGET_FLAGS or lowered in TARGET_EMISSION_FLAGS:
            index += 1
            continue
        if lowered in TARGET_OPTIONS_WITH_VALUE:
            if index + 1 >= len(tokens):
                raise ValueError(f"target option has no value: {token}")
            index += 2
            continue
        adapted.append(token)
        index += 1
    adapted.append("-cuda")
    return " ".join(adapted)


def _load_discovery_workloads(
    tests_dir: Path,
    manifest_path: Path,
    frozen_sources: set[str],
    census: ModuleType,
) -> tuple[list[dict[str, object]], dict[str, int]]:
    manifest_rows = _read_tsv(manifest_path)
    if not 50 <= len(manifest_rows) <= 100:
        raise SystemExit(
            f"discovery manifest must contain 50--100 workloads, found {len(manifest_rows)}"
        )

    workloads: list[dict[str, object]] = []
    selected_sources: set[str] = set()
    tag_counts: dict[str, int] = {}
    for manifest_row in manifest_rows:
        source = manifest_row["source"]
        lowered_source = source.lower()
        if lowered_source in selected_sources:
            raise SystemExit(f"duplicate discovery source: {source}")
        if lowered_source in frozen_sources:
            raise SystemExit(f"discovery source overlaps frozen corpus v1: {source}")
        source_path = tests_dir / Path(source)
        if not source_path.is_file():
            raise SystemExit(f"missing discovery source: {source_path}")
        try:
            source_test_ordinal = int(manifest_row["source_test_ordinal"])
        except ValueError as error:
            raise SystemExit(f"invalid source-test ordinal for {source}") from error
        try:
            directive = _find_compare_directive(
                census._read_text(source_path),
                source_test_ordinal,
            )
            adapted_arguments = _adapt_arguments_to_cuda(str(directive["arguments"]))
        except ValueError as error:
            raise SystemExit(f"invalid discovery contract for {source}: {error}") from error

        tags = [tag.strip() for tag in manifest_row["selection_tags"].split(",") if tag.strip()]
        for tag in tags:
            tag_counts[tag] = tag_counts.get(tag, 0) + 1
        selected_sources.add(lowered_source)
        workloads.append(
            {
                "id": f"{source}#discovery-1",
                "source": source,
                "source_line": directive["line"],
                "source_test_ordinal": source_test_ordinal,
                "cuda_ordinal": 1,
                "categories": directive["categories"],
                "command": directive["command"],
                "arguments": adapted_arguments,
                "original_arguments": directive["arguments"],
                "reference_derived_from_direct": False,
                "coverage_tier": "discovery",
                "scope_reason": manifest_row["rationale"],
                "capability": "cuda_sm_7_0",
                "selection_tags": ",".join(tags),
            }
        )

    missing_tags = REQUIRED_SELECTION_TAGS - set(tag_counts)
    if missing_tags:
        raise SystemExit(
            "discovery manifest does not represent required semantic combinations: "
            + ", ".join(sorted(missing_tags))
        )
    return workloads, dict(sorted(tag_counts.items()))


def _write_selection_files(
    output_root: Path,
    workloads: list[dict[str, object]],
    tag_counts: dict[str, int],
    frozen_metrics: dict[str, int],
    modes: list[str],
    census: ModuleType,
) -> None:
    fields = [
        "id",
        "source",
        "source_line",
        "source_test_ordinal",
        "cuda_ordinal",
        "selection_tags",
        "coverage_tier",
        "scope_reason",
        "categories",
        "command",
        "original_arguments",
        "arguments",
        "capability",
        "reference_derived_from_direct",
    ]
    census._write_tsv(output_root / "selected-workloads.tsv", workloads, fields)
    selection = {
        "schema": 1,
        "corpus": "rolling discovery corpus",
        "selected_workload_count": len(workloads),
        "selected_source_count": len({str(row["source"]) for row in workloads}),
        "source_overlap_with_corpus_v1": 0,
        "frozen_corpus_v1": frozen_metrics,
        "selection_tag_counts": tag_counts,
        "modes": modes,
        "workloads": workloads,
    }
    census._write_text(output_root / "selection.json", json.dumps(selection, indent=2) + "\n")


def _require_mirror_root(output_root: Path, mirror_root: Path) -> Path:
    expected = (output_root / "mirror").resolve()
    actual = mirror_root.resolve()
    if actual != expected or actual == output_root.resolve():
        raise SystemExit(f"refusing to replace unexpected discovery mirror path: {actual}")
    return actual


def _prepare_mirror_tree(tests_dir: Path, output_root: Path) -> Path:
    mirror_root = _require_mirror_root(output_root, output_root / "mirror")
    if mirror_root.exists():
        shutil.rmtree(mirror_root)
    shutil.copytree(tests_dir, mirror_root, copy_function=os.link)
    return mirror_root


def _populate_mirror_for_mode(
    tests_dir: Path,
    mirror_root: Path,
    workloads: list[dict[str, object]],
    mode: str,
    census: ModuleType,
) -> None:
    for workload in workloads:
        original_source = tests_dir / Path(str(workload["source"]))
        source_lines = census._read_text(original_source).splitlines(keepends=True)
        filtered_lines = [
            line for line in source_lines if not census.EXECUTION_DIRECTIVE_RE.match(line)
        ]
        generated_path = mirror_root / census._generated_relative_path(workload)
        census._write_text(
            generated_path,
            census._directive_for_mode(workload, mode) + "".join(filtered_lines),
        )

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


def parse_arguments(census: ModuleType) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument(
        "--manifest",
        type=Path,
        default=Path("issue-nvvm-backend/discovery-corpus.manifest.tsv"),
    )
    parser.add_argument(
        "--frozen-v1",
        type=Path,
        default=Path("issue-nvvm-backend/census.slice-146.tsv"),
    )
    parser.add_argument("--output", type=Path, default=Path("build/nvvm-discovery"))
    parser.add_argument("--jobs", type=int, default=8)
    parser.add_argument("--discover-only", action="store_true")
    parser.add_argument(
        "--classify-only",
        action="store_true",
        help="Reclassify preserved logs without rerunning tests.",
    )
    parser.add_argument("--keep-mirrors", action="store_true")
    parser.add_argument(
        "--match",
        help="Run only workload IDs containing this case-insensitive substring.",
    )
    parser.add_argument(
        "--modes",
        nargs="+",
        choices=sorted(census.MODES),
        default=list(census.MODES),
    )
    return parser.parse_args()


def main() -> int:
    script_dir = Path(__file__).resolve().parent
    census = _load_census_module(script_dir)
    _install_discovery_classifier(census)
    args = parse_arguments(census)
    repo_root = args.repo.resolve()
    tests_dir = repo_root / "tests"
    output_root = _resolve_path(repo_root, args.output)
    manifest_path = _resolve_path(repo_root, args.manifest)
    frozen_v1_path = _resolve_path(repo_root, args.frozen_v1)
    frozen_sources, frozen_metrics = _audit_frozen_v1(frozen_v1_path)
    workloads, tag_counts = _load_discovery_workloads(
        tests_dir,
        manifest_path,
        frozen_sources,
        census,
    )
    output_root.mkdir(parents=True, exist_ok=True)
    _write_selection_files(
        output_root,
        workloads,
        tag_counts,
        frozen_metrics,
        args.modes,
        census,
    )
    print(
        f"selected {len(workloads)} discovery workloads with zero corpus-v1 source overlap",
        flush=True,
    )
    if args.discover_only:
        return 0

    provider_path = repo_root / "build/nvvm-builder-deps/slang-llvm-nvvm-build/Release"
    test_runner = repo_root / "build/Release/bin/slang-test.exe"
    if not test_runner.is_file():
        raise SystemExit(f"missing Release test runner: {test_runner}")
    if not provider_path.is_dir():
        raise SystemExit(f"missing NVVM provider directory: {provider_path}")

    if args.classify_only:
        results_path = output_root / "results.json"
        if not results_path.is_file():
            raise SystemExit(f"missing prior discovery results: {results_path}")
        results = json.loads(census._read_text(results_path))
        for result in results:
            log_path = repo_root / Path(str(result["log"]))
            classification, diagnostic, shape = census._classify_result(
                int(result["return_code"]),
                census._read_text(log_path),
                str(result["mode"]),
            )
            result["classification"] = classification
            result["diagnostic"] = diagnostic
            result["canonical_shape"] = shape
        counts = census._write_result_files(output_root, results)
        print(json.dumps(counts, indent=2), flush=True)
        return 0 if all("unclassified" not in item for item in counts.values()) else 2

    run_workloads = workloads
    if args.match:
        needle = args.match.lower()
        run_workloads = [row for row in workloads if needle in str(row["id"]).lower()]
        if not run_workloads:
            raise SystemExit(f"no discovery workload ID contains: {args.match}")
        print(f"selected {len(run_workloads)} workloads matching {args.match!r}", flush=True)

    print("preparing one hard-linked discovery mirror", flush=True)
    mirror_root = _prepare_mirror_tree(tests_dir, output_root)
    all_results: list[dict[str, object]] = []
    for mode in args.modes:
        print(f"preparing generated discovery {mode} lanes", flush=True)
        _populate_mirror_for_mode(tests_dir, mirror_root, run_workloads, mode, census)
        all_results.extend(
            census.run_mode(
                repo_root,
                output_root,
                mirror_root,
                run_workloads,
                mode,
                provider_path,
                args.jobs,
            )
        )

    if not args.keep_mirrors:
        shutil.rmtree(_require_mirror_root(output_root, mirror_root))

    counts = census._write_result_files(output_root, all_results)
    print(json.dumps(counts, indent=2), flush=True)
    return 0 if all("unclassified" not in item for item in counts.values()) else 2


if __name__ == "__main__":
    sys.exit(main())
