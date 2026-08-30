#!/usr/bin/env python3

"""Pivot raw compute-census results into a per-workload TSV evidence table."""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path
import re


AGGREGATE_SHAPES = {
    "struct field address",
    "local resource-struct layout",
    "sequential element pointer",
    "basic-block parameter",
    "makeStruct",
    "structured-buffer aggregate layout",
    "entry-point parameter",
    "call argument type",
    "global_param",
}


NUMERIC_SHAPES = {
    "intCast",
    "shr",
    "bitfieldInsert",
    "bitfieldExtract",
    "castIntToFloat",
    "bitCast",
}


ATOMIC_WAVE_SHAPES = {
    "waveMaskMatch",
    "atomicCompareExchange",
    "atomicExchange",
    "atomicAnd",
    "atomicStore",
    "selected atomic operation",
}


RAW_BUFFER_SHAPES = {
    "equivalent structured-buffer view",
    "core byte-address buffer access",
}


RESIDUAL_SHAPES = {
    "LoadFromUninitializedMemory",
    "DebugNoScope",
    "RequireComputeDerivative",
    "RequirePrelude",
    "getStringHash",
    "RequireMaximallyReconverges",
}


EXTENSION_WAVE_RE = re.compile(
    r"^(?:hlsl-intrinsic/wave-rotate/|hlsl-intrinsic/wave-multi/|"
    r"hlsl-intrinsic/wave-prefix-|hlsl-intrinsic/wave-mask/wave-(?:mask-)?prefix-|"
    r"hlsl-intrinsic/quad-control/)|(?:^|/)wave-matrix\.slang$",
    re.IGNORECASE,
)


def _coverage_tier(source: str) -> tuple[str, str]:
    if EXTENSION_WAVE_RE.search(source):
        return "extension", "advanced wave/quad operation outside the initial MVP"
    if source.lower() == "slang-extension/realtime-clock.slang":
        return "extension", "device clock/syscall operation outside the initial MVP"
    return "mvp", ""


def _generic_asm_cluster(source: str) -> tuple[str, str]:
    producer = (
        "StmtLoweringVisitor::visitIntrinsicAsmStmt -> IRGenericAsm; "
        "_validateNVVMFunction semantic resolution"
    )
    if re.search(r"wave|divergence|reconvergence", source, re.IGNORECASE):
        return "generic-asm-wave-reconvergence", producer
    if re.search(r"atomic|cas-int64", source, re.IGNORECASE):
        return "generic-asm-atomic", producer
    if "texture" in source.lower():
        return "generic-asm-texture", producer
    if "realtime-clock" in source.lower():
        return "generic-asm-device-clock", producer
    return "generic-asm-ordinary-intrinsic", producer


def _runtime_cluster(source: str) -> tuple[str, str]:
    lowered = source.lower()
    producer = "compiled PTX executed by the CUDA compare-compute harness"
    if "/conversions/conversion-to-" in lowered:
        return "runtime-narrow-integer-conversion-o3", producer
    if "layout-descriptor-handle" in lowered:
        return "runtime-descriptor-handle-layout", producer
    if "cbuffer-float3-offsets-aligned" in lowered:
        return "runtime-constant-buffer-layout", producer
    if "bound-check-zero-index" in lowered:
        return "runtime-bounds-zero-index", producer
    if "anyvalue-layout" in lowered:
        return "runtime-anyvalue-layout", producer
    if "atomic-mixed-width" in lowered:
        return "runtime-sm90-mixed-width-atomic", producer
    return "runtime-other", producer


def _failure_ownership(row: dict[str, str]) -> tuple[str, str]:
    classification = row["classification"]
    source = row["source"]
    shape = row["canonical_shape"]
    if classification == "correct":
        return "", ""
    if classification == "runtime-mismatch":
        return _runtime_cluster(source)
    if classification == "infrastructure":
        return "infrastructure-native-reference", "NVRTC/toolkit or generated reference contract"
    if classification == "provider":
        if "by-value aggregate field pointer" in row["diagnostic"]:
            return (
                "provider-aggregate-field-pointer",
                "canonical by-value aggregate field address -> provider field-pointer operation",
            )
        return (
            "provider-unoptimized-half-operation",
            "NVVMTypeLoweringContext/emitNVVMIRFromLinkedIR -> libNVVM compilation",
        )
    if classification != "preflight":
        return "unclassified", "manual audit required"
    if shape == "GenericAsm":
        return _generic_asm_cluster(source)
    if shape in {"helper function parameter", "helper function result type"}:
        return (
            "helper-abi-type-contract",
            "post-specialization linked IRFunc signature -> _validateNVVMHelperTarget",
        )
    if shape in AGGREGATE_SHAPES:
        return (
            "aggregate-pointer-layout-transport",
            "canonical linked aggregate/pointer IR -> _validateNVVMFunction",
        )
    if shape in NUMERIC_SHAPES:
        return (
            "ordinary-numeric-bit-operation",
            "typed expression/intrinsic IR op -> _resolveNVVMValueOperation",
        )
    if shape in ATOMIC_WAVE_SHAPES:
        return (
            "atomic-wave-operation",
            "atomic/wave legalization IR op -> _validateNVVMFunction",
        )
    if shape in RAW_BUFFER_SHAPES:
        return (
            "raw-buffer-view-access",
            "raw-buffer legalization IR op -> direct-NVVM raw-buffer resolver",
        )
    if shape == "function name":
        return (
            "function-identity",
            "post-specialization linked IRFunc -> _collectNVVMFunctionNames",
        )
    if shape in RESIDUAL_SHAPES:
        return (
            "residual-target-marker-or-undefined-value",
            "named upstream IR producer -> _validateNVVMFunction default rejection",
        )
    return "preflight-other", "canonical linked IR -> _validateNVVMFunction"


def _failure_detail(row: dict[str, str], repo_root: Path) -> str:
    if row["classification"] == "correct":
        return ""
    if row["diagnostic"]:
        return row["diagnostic"]
    log_text = (repo_root / row["log"]).read_text(
        encoding="utf-8",
        errors="replace",
    )
    filecheck = re.search(r"slang-test:.*?: error: ([^\r\n]+)", log_text)
    if filecheck:
        return filecheck.group(1).strip()
    if "libNVVM compilation failed" in log_text:
        return "libNVVM compilation failed: Error: unsupported operation"
    nvrtc_error = re.search(r"nvrtc[^\r\n]*?(error\s*:[^\r\n]+)", log_text, re.IGNORECASE)
    if nvrtc_error:
        return nvrtc_error.group(1).strip()
    warning = re.search(r"warning\[E\d+\]:[^\r\n]+", log_text)
    if warning:
        return warning.group(0).strip()
    if "EXPECTED{{{" in log_text and "ACTUAL{{{" in log_text:
        return "slang-test expected/actual runtime result mismatch"
    return "see preserved raw census log"


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--input", type=Path, default=Path("build/nvvm-census/results.tsv"))
    parser.add_argument(
        "--table",
        type=Path,
        default=Path("build/nvvm-census/census.slice-131.tsv"),
        help="destination for the generated per-workload TSV table",
    )
    parser.add_argument(
        "--clusters",
        type=Path,
        default=Path("build/nvvm-census/clusters.slice-131.json"),
    )
    return parser.parse_args()


def main() -> int:
    args = parse_arguments()
    repo_root = args.repo.resolve()
    input_path = repo_root / args.input if not args.input.is_absolute() else args.input
    output_path = repo_root / args.table if not args.table.is_absolute() else args.table
    cluster_path = repo_root / args.clusters if not args.clusters.is_absolute() else args.clusters
    with input_path.open("r", encoding="utf-8", newline="") as stream:
        raw_rows = list(csv.DictReader(stream, delimiter="\t"))
    by_id: dict[str, dict[str, dict[str, str]]] = {}
    for row in raw_rows:
        by_id.setdefault(row["id"], {})[row["mode"]] = row

    output_rows: list[dict[str, str]] = []
    cluster_counts: dict[str, dict[str, int]] = {"nvrtc-o3": {}, "nvvm-o0": {}, "nvvm-o3": {}}
    for workload_id in sorted(by_id):
        modes = by_id[workload_id]
        native = modes["nvrtc-o3"]
        coverage_tier, scope_reason = _coverage_tier(native["source"])
        row: dict[str, str] = {
            "id": workload_id,
            "source": native["source"],
            "capability": native["capability"],
            "reference_derived_from_direct": native["reference_derived_from_direct"],
            "coverage_tier": coverage_tier,
            "scope_reason": scope_reason,
        }
        for mode in ("nvrtc-o3", "nvvm-o0", "nvvm-o3"):
            result = modes[mode]
            cluster, producer = _failure_ownership(result)
            detail = _failure_detail(result, repo_root)
            row[f"{mode}_classification"] = result["classification"]
            row[f"{mode}_elapsed_ms"] = result["elapsed_ms"]
            row[f"{mode}_shape"] = result["canonical_shape"]
            row[f"{mode}_cluster"] = cluster
            row[f"{mode}_producer"] = producer
            row[f"{mode}_diagnostic"] = detail
            if cluster:
                counts = cluster_counts[mode]
                counts[cluster] = counts.get(cluster, 0) + 1
        row["evidence_status"] = "complete"
        output_rows.append(row)

    fields = list(output_rows[0])
    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields, delimiter="\t", lineterminator="\n")
        writer.writeheader()
        writer.writerows(output_rows)
    cluster_path.parent.mkdir(parents=True, exist_ok=True)
    cluster_path.write_text(
        json.dumps(cluster_counts, indent=2) + "\n",
        encoding="utf-8",
        newline="\n",
    )
    print(f"wrote {len(output_rows)} workload rows to {output_path}")
    print(json.dumps(cluster_counts, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
