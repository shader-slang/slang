#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Measure original corpus dispatches, retaining ordinary test oracles and every failed cell.

CUDA event intervals include the RHI global-parameter upload and host enqueue gaps. They are
not kernel-only throughput measurements. Compilation, binding, reset and readback are excluded.
Run without concurrent builds, GPU work or CPU-heavy reviews. Each invocation needs a new output.
"""

import argparse
import collections
import csv
import hashlib
import importlib.util
import json
import math
from pathlib import Path
import re
import statistics
import subprocess
import sys

REPO = Path(__file__).resolve().parents[1]
HERE = REPO / "issue-nvvm-backend"
MODES = ("nvrtc-o3", "nvvm-o0", "nvvm-o3")
SCOPE = "cuda-dispatch-including-global-parameter-upload-and-host-enqueue-gaps"
ROUNDS, WARMUPS, SAMPLES = 2, 3, 9
RATIO_MIN_MS = 0.1


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def save(path, value):
    Path(path).write_text(json.dumps(value, indent=2, allow_nan=False) + "\n")


def validate_profile(rows, warmups=WARMUPS, samples=SAMPLES):
    """Reject truncated, reordered, failed, nonfinite or incompletely checked sidecars."""
    expected = [("reference", 0)] + [("warmup", i) for i in range(warmups)] + [
        ("sample", i) for i in range(samples)]
    if len(rows) != len(expected) + 3:
        raise ValueError("incomplete profile inventory")
    header, resources, *tail = rows
    if header != dict(record="header", schema_version=1, scope=SCOPE,
                      timing="cuda-event-ms", warmups=warmups, samples=samples):
        raise ValueError("unexpected timing contract")
    if resources.get("record") != "resources" or any(
        type(resources.get(key)) is not int or resources[key] < minimum
        for key, minimum in (("allocations", 1), ("snapshots", 1), ("reset_bytes", 1), ("outputs", 1))
    ):
        raise ValueError("missing resource reset/output inventory")
    launches, summary = tail[:-1], tail[-1]
    if summary != dict(record="summary", status="completed", launches=len(expected),
                       warmups=warmups, samples=samples):
        raise ValueError("profile did not complete")
    for row, key in zip(launches, expected):
        if row.get("record") != "launch" or type(row.get("index")) is not int or (row.get("phase"), row.get("index")) != key:
            raise ValueError("launch order differs from protocol")
        value = row.get("device_ms")
        if type(value) not in (int, float) or not math.isfinite(value) or value < 0:
            raise ValueError("invalid CUDA event time")
        if row.get("output_equal") is not True or any(type(row.get(k)) is not int or row[k] != 1 for k in ("start_callbacks", "end_callbacks")):
            raise ValueError("launch lacks checked output or event callbacks")
    return [row["device_ms"] for row in launches if row["phase"] == "sample"]


def parse_batch(output, names, process, census):
    """Require a unique result for each requested test and a consistent authoritative summary.

    A batch with ordinary test failures still supplies valid per-test results. A timeout, lost
    server, unexpected test, missing summary or invalid process exit invalidates the whole batch.
    """
    plain = census.normalize_diagnostic_output(output)
    matches = re.findall(r"^(passed|FAILED|ignored|failed\(expected\)|UNEXPECTED) test: '([^']+)'[^\n]*$", plain, re.M)
    if len(matches) != len(names) or len({name for _, name in matches}) != len(names):
        raise ValueError("missing or duplicate test results")
    if [name for _, name in matches] != names:
        raise ValueError("executed tests differ from requested inventory or order")
    statuses = dict((name, status) for status, name in matches)
    if any(status not in ("passed", "FAILED") for status in statuses.values()):
        raise ValueError("skipped or unexpected test status")
    passed = sum(status == "passed" for status in statuses.values())
    expected = dict(passed=passed, executed=len(names), ignored=0, other_summary_status=0)
    if census.execution_counts(plain) != expected:
        raise ValueError("test summary does not match individual results")
    if process["timed_out"] or process["return_code"] != (0 if passed == len(names) else 1):
        raise ValueError("unexpected process outcome")
    if re.search(r"test.server (?:loss|dispatch failure)", plain, re.I):
        raise ValueError("test server infrastructure failure")
    return statuses


def select(census, discovery, case_ids):
    workloads, _ = census.discover_workloads(REPO / "tests")
    frozen = census.select_frozen_workloads(workloads, HERE / "census.slice-195.tsv")
    sources, _ = discovery._audit_frozen_v1(HERE / "census.slice-146.tsv")
    discovered, _ = discovery._load_discovery_workloads(
        REPO / "tests", HERE / "discovery-corpus.manifest.tsv", sources, census)
    selected = []
    for corpus, rows in (("frozen", frozen), ("discovery", discovered)):
        census.select_architecture(rows, 80)
        selected.extend(dict(row, corpus=corpus) for row in rows)
    if len({row["id"] for row in selected}) != len(selected):
        raise ValueError("duplicate corpus case")
    if case_ids:
        if set(case_ids) - {row["id"] for row in selected}:
            raise ValueError("unknown requested case")
        selected = [row for row in selected if row["id"] in case_ids]
    return selected


def expected_schedule(rows):
    return [(row["id"], mode, rd) for rd in range(ROUNDS)
            for mode in (MODES if rd == 0 else tuple(reversed(MODES)))
            for row in (rows if rd == 0 else list(reversed(rows)))]


def prepare_mirror(census, output, rows, mode, round_index):
    parent = output / f"round-{round_index}"
    root = parent / "mirrors" / mode
    census.prepare_mode(REPO / "tests", parent, root, rows, mode)
    sidecars = parent / (mode + "-profiles")
    sidecars.mkdir()
    cells = []
    for row in rows:
        generated = root / census._generated_relative_path(row)
        key = hashlib.sha256(row["id"].encode()).hexdigest()[:20]
        profile = sidecars / (key + ".jsonl")
        directive, source = generated.read_text().split("\n", 1)
        # The renderer extracts uppercase -Xslang first, then appends legacy options.
        # End with legacy overrides so original -compile-arg -O3/-xslang -g2 cannot win.
        backend, optimization = mode.split("-o")
        directive += (f' -g0 -compile-arg -g0 -compile-arg -O{optimization}'
                      f' -compile-arg -emit-cuda-via-{backend} -cuda-dispatch-profile {profile}'
                      f" -cuda-dispatch-warmups {WARMUPS} -cuda-dispatch-samples {SAMPLES}")
        generated.write_text(directive + "\n" + source)
        cells.append(dict(id=row["id"], corpus=row["corpus"], source=row["source"], mode=mode,
                          round=round_index, generated=str(generated), profile=str(profile)))
    return root, cells


def collect(args):
    nr = load("nvvm_results", HERE / "nvvm-results.py")
    census = load("census", HERE / "run-compute-census.py")
    discovery = load("discovery", HERE / "run-compute-discovery.py")
    rows = select(census, discovery, args.case)
    args.output = args.output.resolve()
    args.output.relative_to(REPO)
    if any(char.isspace() or char in "\"\'" for char in str(args.output)):
        raise ValueError("test directives require an output path without whitespace or quotes")
    args.output.mkdir(parents=True, exist_ok=False)
    args.command = "corpus-runtime"
    _, environment, provenance = nr.configure(args, args.output)
    extra = [Path(__file__).resolve(), HERE / "run-compute-census.py", HERE / "run-compute-discovery.py",
             HERE / "census.slice-195.tsv", HERE / "census.slice-146.tsv",
             HERE / "discovery-corpus.manifest.tsv", args.baseline.resolve()]
    extra.extend((REPO / "tools/render-test").glob("*"))
    extra.extend((REPO / "build").glob("CMakeCache.txt"))
    provenance["artifact_sha256"].update({str(p): nr.sha(p) for p in extra if p.is_file()})
    patch = subprocess.check_output(["git", "diff", "HEAD", "--", "tools/render-test", "tests", "extras", "issue-nvvm-backend"], cwd=REPO)
    (args.output / "harness.diff").write_bytes(patch)
    save(args.output / "provenance.json", provenance)
    accepted = nr.accepted_baseline(args.baseline)
    before = nr.index_outcomes([r for corpus in accepted["corpora"].values() for r in corpus["fresh_cell_outcomes"]])
    if not args.case and {row["id"] for row in rows} != {key[0] for key in before}:
        raise ValueError("full selection differs from accepted baseline inventory")
    selected = [dict(row, source_sha256=nr.sha(REPO / "tests" / row["source"]),
                     accepted_outcomes={mode: before[row["id"], mode] for mode in MODES}) for row in rows]
    manifest = dict(schema=1, kind="corpus-dispatch-performance", scope=SCOPE,
                    selection="subset" if args.case else "full-frozen-and-discovery",
                    rounds=ROUNDS, warmups=WARMUPS, samples=SAMPLES, debug_info="g0",
                    ratio_min_median_ms=RATIO_MIN_MS, batch_size=args.batch_size,
                    baseline=nr.reference(args.baseline), provenance=nr.reference(args.output / "provenance.json"), selected=selected,
                    interpretation="Original-input dispatch latency; short intervals are observations, not reliable speedups.")
    mirrors, cells = [], []
    for rd in range(ROUNDS):
        ordered = rows if rd == 0 else list(reversed(rows))
        for mode in MODES if rd == 0 else reversed(MODES):
            root, group = prepare_mirror(census, args.output, ordered, mode, rd)
            mirrors.append((root, group))
            cells.extend(group)
    if [(row["id"], row["mode"], row["round"]) for row in cells] != expected_schedule(rows):
        raise ValueError("incorrect measurement schedule")
    manifest["cells"] = cells
    manifest["mirror_input_sha256"] = {str(path): nr.sha(path) for root, _ in mirrors
                                        for path in sorted(root.rglob("*")) if path.is_file()}
    save(args.output / "manifest.json", manifest)
    report = dict(schema=1, status="running", manifest=nr.reference(args.output / "manifest.json"),
                  batches=[], cells=[])
    save(args.output / "results.json", report)
    batch_index = 0
    for root, group in mirrors:
        for start in range(0, len(group), args.batch_size):
            batch = group[start:start + args.batch_size]
            paths = [str(Path(row["generated"]).relative_to(REPO)) for row in batch]
            names = [path + " (cuda)" for path in paths]
            command = [args.slangc.parent / "slang-test", "-test-dir", root, "-use-test-server",
                       "-server-count", "1", "-disable-retries", "-explicit-test-order", "-v", "info", *paths]
            process = nr.run(command, args.output / f"batch-{batch_index:04d}.log", environment, 1800)
            batch_index += 1
            process["names"] = names
            process["log_sha256"] = nr.sha(Path(process["log"]))
            output = Path(process["log"]).read_text(errors="replace")
            try:
                statuses = parse_batch(output, names, process, census)
                batch_error = None
            except ValueError as error:
                statuses, batch_error = {}, str(error)
            process["inventory_error"] = batch_error
            report["batches"].append(process)
            for cell, name in zip(batch, names):
                result = dict(cell, log=process["log"], oracle=statuses.get(name, "unverified"))
                profile = Path(cell["profile"])
                actual = Path(cell["generated"] + ".actual.txt")
                if actual.is_file():
                    result["reference_output"] = nr.reference(actual)
                if profile.is_file():
                    result["profile_sha256"] = nr.sha(profile)
                try:
                    records = [json.loads(line) for line in profile.read_text().splitlines()]
                    times = validate_profile(records)
                    if statuses.get(name) != "passed" or not actual.is_file():
                        raise ValueError("original test oracle did not pass")
                    result.update(status="measured", sample_ms=times)
                except (ValueError, OSError) as error:
                    result.update(status="excluded", reason=batch_error or str(error))
                report["cells"].append(result)
            save(args.output / "results.json", report)
            print(f'{len(report["cells"])}/{len(cells)} cells; '
                  f'{sum(r["status"] == "measured" for r in report["cells"])} measured; '
                  f'{process["elapsed_seconds"]:.1f}s batch', flush=True)
            if batch_error:
                report["status"] = "infrastructure-failed"
                save(args.output / "results.json", report)
                raise ValueError("batch inventory failed: " + batch_error)
    nr.verify_identity(provenance)
    for path, digest in manifest["mirror_input_sha256"].items():
        if nr.sha(Path(path)) != digest:
            raise ValueError("mirror input changed during execution")
    report["status"] = "completed-with-explicit-exclusions" if any(
        row["status"] != "measured" for row in report["cells"]) else "completed"
    save(args.output / "results.json", report)
    summarize(args.output)


def summarize(root):
    """Export all cells and paired observations without silently dropping short intervals."""
    nr = load("nvvm_results_report", HERE / "nvvm-results.py")
    report = nr.read(root / "results.json")
    manifest = nr.read(root / "manifest.json")
    if report["manifest"]["sha256"] != nr.sha(root / "manifest.json"):
        raise ValueError("manifest changed")
    key = lambda row: (row["id"], row["mode"], row["round"])
    expected = expected_schedule(manifest["selected"])
    if [key(row) for row in manifest["cells"]] != expected:
        raise ValueError("manifest does not cover selected cases, modes and rounds")
    provenance = manifest["provenance"]
    if nr.sha(Path(provenance["path"])) != provenance["sha256"]:
        raise ValueError("provenance record changed")
    for path, digest in manifest["mirror_input_sha256"].items():
        if nr.sha(Path(path)) != digest:
            raise ValueError("mirrored input or oracle changed")
    if report["status"] not in ("completed", "completed-with-explicit-exclusions") or [key(row) for row in report["cells"]] != expected or len(set(expected)) != len(expected):
        raise ValueError("incomplete or duplicate measurement inventory")
    census = load("report_census", HERE / "run-compute-census.py")
    oracle = {}
    oracle_logs = {}
    for batch in report["batches"]:
        path = Path(batch["log"])
        if nr.sha(path) != batch["log_sha256"]:
            raise ValueError("test log changed")
        statuses = parse_batch(path.read_text(errors="replace"), batch["names"], batch, census)
        if oracle.keys() & statuses.keys():
            raise ValueError("duplicate executed test across batches")
        oracle.update(statuses)
        oracle_logs.update({name: batch["log"] for name in statuses})
    names = {str(Path(row["generated"]).relative_to(REPO)) + " (cuda)" for row in manifest["cells"]}
    if oracle.keys() != names:
        raise ValueError("batch inventory differs from manifest")
    groups = collections.defaultdict(list)
    for row, contract in zip(report["cells"], manifest["cells"]):
        if any(row.get(field) != value for field, value in contract.items()):
            raise ValueError("result artifact binding differs from manifest")
        name = str(Path(row["generated"]).relative_to(REPO)) + " (cuda)"
        if row["oracle"] != oracle[name] or row["log"] != oracle_logs[name]:
            raise ValueError("cell oracle differs from actual test log")
        profile = Path(row["profile"])
        if profile.is_file() != ("profile_sha256" in row):
            raise ValueError("sidecar presence changed")
        if profile.is_file() and nr.sha(profile) != row["profile_sha256"]:
            raise ValueError("sidecar changed")
        actual = Path(row["generated"] + ".actual.txt")
        if actual.is_file() != ("reference_output" in row):
            raise ValueError("reference presence changed")
        if actual.is_file():
            if row["reference_output"] != nr.reference(actual):
                raise ValueError("reference output binding or bytes changed")
        try:
            times = validate_profile([json.loads(line) for line in profile.read_text().splitlines()])
            measured = row["oracle"] == "passed" and actual.is_file()
        except (OSError, ValueError):
            measured = False
        if row["status"] != ("measured" if measured else "excluded"):
            raise ValueError("recorded classification differs from artifacts")
        if row["status"] == "measured":
            reference = row["reference_output"]
            if nr.sha(Path(reference["path"])) != reference["sha256"]:
                raise ValueError("oracle reference output changed")
            profile = Path(row["profile"])
            if nr.sha(profile) != row["profile_sha256"] or row["oracle"] != "passed":
                raise ValueError("unqualified measured cell")
            samples = validate_profile([json.loads(line) for line in profile.read_text().splitlines()])
            if samples != row["sample_ms"]:
                raise ValueError("sample values changed")
        groups[row["id"], row["mode"]].append(row)
    table = []
    for selected in manifest["selected"]:
        for mode in MODES:
            rows = groups[selected["id"], mode]
            complete = len(rows) == ROUNDS and all(row["status"] == "measured" for row in rows)
            values = [value for row in rows for value in row.get("sample_ms", [])] if complete else []
            summary = dict(id=selected["id"], corpus=selected["corpus"], source=selected["source"], mode=mode,
                           status="measured" if complete else "excluded", samples=len(values),
                           median_ms=statistics.median(values) if values else None,
                           round0_median_ms=statistics.median(rows[0]["sample_ms"]) if complete else None,
                           round1_median_ms=statistics.median(rows[1]["sample_ms"]) if complete else None,
                           min_ms=min(values) if values else None, max_ms=max(values) if values else None,
                           reasons="; ".join(sorted({row.get("reason", "") for row in rows} - {""})))
            table.append(summary)
    with (root / "per-case.csv").open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(table[0]))
        writer.writeheader()
        writer.writerows(table)
    lookup = {(row["id"], row["mode"]): row for row in table}
    pairs = []
    for selected in manifest["selected"]:
        a, b, c = [lookup[selected["id"], mode] for mode in MODES]
        complete = all(row["status"] == "measured" for row in (a, b, c))
        interpretable = complete and min(row["median_ms"] for row in (a, b, c)) >= RATIO_MIN_MS
        pairs.append(dict(id=selected["id"], corpus=selected["corpus"], status=(
            "ratio-eligible" if interpretable else "short-interval" if complete else "incomplete"),
            nvrtc_o3_ms=a["median_ms"], nvvm_o0_ms=b["median_ms"], nvvm_o3_ms=c["median_ms"],
            nvrtc_over_nvvm_o3=a["median_ms"] / c["median_ms"] if interpretable else None,
            nvrtc_over_nvvm_o0=a["median_ms"] / b["median_ms"] if interpretable else None,
            round0_nvrtc_over_nvvm_o3=(a["round0_median_ms"] / c["round0_median_ms"] if interpretable and min(row["round0_median_ms"] for row in (a,b,c)) >= RATIO_MIN_MS else None),
            round1_nvrtc_over_nvvm_o3=(a["round1_median_ms"] / c["round1_median_ms"] if interpretable and min(row["round1_median_ms"] for row in (a,b,c)) >= RATIO_MIN_MS else None)))
    with (root / "paired.csv").open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(pairs[0]))
        writer.writeheader()
        writer.writerows(pairs)
    with (root / "samples.csv").open("w", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(["id", "corpus", "mode", "round", "index", "device_ms"])
        for row in report["cells"]:
            for index, value in enumerate(row.get("sample_ms", [])):
                writer.writerow([row["id"], row["corpus"], row["mode"], row["round"], index, value])
    summary = dict(schema=1, scope=SCOPE, rounds=ROUNDS, warmups=WARMUPS, samples=SAMPLES,
                   ratio_min_median_ms=RATIO_MIN_MS, mode_round_status=dict(collections.Counter(row["status"] for row in report["cells"])),
                   corpora={corpus: dict(collections.Counter(row["status"] for row in pairs if row["corpus"] == corpus)) for corpus in ("frozen", "discovery")},
                   pairs=pairs)
    save(root / "summary.json", summary)
    print(json.dumps({key: value for key, value in summary.items() if key != "pairs"}, indent=2))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--slangc", type=Path, default=REPO / "build/RelWithDebInfo/bin/slangc")
    parser.add_argument("--provider", type=Path, default=REPO / "build/RelWithDebInfo/bin/libslang-llvm-nvvm.so")
    parser.add_argument("--cuda-root", type=Path, default=Path("/usr/local/cuda-12.9"))
    parser.add_argument("--build-label", default="RelWithDebInfo")
    parser.add_argument("--baseline", type=Path, default=HERE / "accepted-baseline.json")
    parser.add_argument("--batch-size", type=int, default=32)
    parser.add_argument("--case", action="append", help="Exact corpus case ID; omitted selects both full corpora.")
    parser.add_argument("--report-only", action="store_true")
    args = parser.parse_args()
    if not 1 <= args.batch_size <= 128:
        parser.error("batch size must be in 1..128")
    try:
        if args.report_only:
            summarize(args.output.resolve())
        else:
            collect(args)
    except (ValueError, OSError) as error:
        print("error: " + str(error), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
