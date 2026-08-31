#!/usr/bin/env python3

"""Summarize the rolling discovery corpus separately from frozen corpus v1."""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path
import re


MODES = ("nvrtc-o3", "nvvm-o0", "nvvm-o3")
PREFLIGHT_EXACT_SHAPE_RE = re.compile(
    r"direct NVVM lowering does not support Slang IR instruction or shape '(.+)'$"
)
PROVIDER_OPERATION_RE = re.compile(r"builder operation '([^']+)'")
EXPECTED_ACTUAL_RE = re.compile(
    r"EXPECTED\{\{\{\s*(.*?)\s*\}\}\}\s*ACTUAL\{\{\{\s*(.*?)\s*\}\}\}",
    re.DOTALL,
)
NVRTC_ERROR_RE = re.compile(r"^(nvrtc[^\r\n]*error[^\r\n]*)", re.IGNORECASE | re.MULTILINE)


def _resolve_path(repo_root: Path, path: Path) -> Path:
    return path.resolve() if path.is_absolute() else (repo_root / path).resolve()


def _read_tsv(path: Path) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8", newline="") as stream:
        return list(csv.DictReader(stream, delimiter="\t"))


def _write_tsv(path: Path, rows: list[dict[str, object]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(
            stream,
            fieldnames=list(rows[0]),
            delimiter="\t",
            lineterminator="\n",
        )
        writer.writeheader()
        writer.writerows(rows)


def _read_log(repo_root: Path, row: dict[str, str]) -> str:
    return (repo_root / Path(row["log"])).read_text(encoding="utf-8", errors="replace")


def _compact_output(text: str, limit: int = 180) -> str:
    compact = " | ".join(line.strip() for line in text.splitlines() if line.strip())
    return compact if len(compact) <= limit else compact[: limit - 3] + "..."


def _exact_shape(row: dict[str, str], log_text: str) -> str:
    classification = row["classification"]
    if classification == "correct":
        return ""
    if classification == "preflight":
        match = PREFLIGHT_EXACT_SHAPE_RE.search(row["diagnostic"])
        if not match:
            raise ValueError(f"preflight row has no exact shape: {row['id']} {row['mode']}")
        return match.group(1)
    if classification == "provider":
        match = PROVIDER_OPERATION_RE.search(row["diagnostic"])
        return f"provider operation: {match.group(1)}" if match else "LLVM/libNVVM provider"
    if classification == "infrastructure":
        if row["canonical_shape"]:
            return row["canonical_shape"]
        if NVRTC_ERROR_RE.search(log_text):
            return "NVRTC CUDA C++ compilation"
        return "CUDA test infrastructure"
    if classification == "runtime-mismatch":
        return "runtime result buffer (no unsupported canonical IR shape)"
    raise ValueError(f"unclassified discovery result: {row['id']} {row['mode']}")


def _failure_diagnostic(row: dict[str, str], log_text: str) -> str:
    if row["classification"] == "correct":
        return ""
    if row["diagnostic"]:
        return row["diagnostic"]
    nvrtc_match = NVRTC_ERROR_RE.search(log_text)
    if nvrtc_match:
        return nvrtc_match.group(1).strip()
    mismatch = EXPECTED_ACTUAL_RE.search(log_text)
    if mismatch:
        return (
            f"expected [{_compact_output(mismatch.group(1))}], "
            f"actual [{_compact_output(mismatch.group(2))}]"
        )
    return "see preserved discovery log"


def _runtime_cluster(tags: str) -> tuple[str, str]:
    tag_set = {tag.strip() for tag in tags.split(",")}
    if "matrix-layout" in tag_set:
        return (
            "runtime-matrix-layout",
            "NVRTC/direct-LLVM PTX execution -> compare-compute result buffer",
        )
    if "mixed-resources" in tag_set:
        return (
            "runtime-resource-query",
            "NVRTC/direct-LLVM PTX resource execution -> compare-compute result buffer",
        )
    return "runtime-other", "compiled PTX execution -> compare-compute result buffer"


def _preflight_cluster(exact_shape: str) -> tuple[str, str]:
    if exact_shape.startswith("struct field address"):
        return (
            "aggregate-struct-field-pointer",
            "IRBuilder::emitFieldAddress -> IRFieldAddress; _validateNVVMFunction",
        )
    if exact_shape == "aggregate storage layout":
        return (
            "aggregate-storage-layout",
            "entry-point/buffer type lowering -> conventional global aggregate; "
            "_hasNVVMCompatibleAggregateStorageLayout",
        )
    if exact_shape == "array element pointer relation":
        return (
            "aggregate-array-element-pointer",
            "IRBuilder::emitElementAddress -> IRGetElementPtr; _validateNVVMFunction",
        )
    if exact_shape.startswith("sequential element pointer:"):
        return (
            "aggregate-sequential-pointer",
            "IRBuilder::emitElementAddress -> typed IRGetElementPtr; "
            "_getNVVMSequentialElementPointer",
        )
    if exact_shape.startswith("device scalar pointer:"):
        return (
            "device-pointer-load-chain",
            "entry-point/parameter-group pointer lowering -> IRLoad pointer operand; "
            "_validateNVVMPointerValue",
        )
    if exact_shape == "entry-point parameter":
        return (
            "entry-point-parameter-abi",
            "collectEntryPointUniforms and specialization -> linked IRFunc parameter; "
            "_validateNVVMFunction",
        )
    if exact_shape == "function name" or exact_shape.startswith("duplicate function name: "):
        return (
            "function-identity",
            "linkage/specialization function decorations -> _getNVVMFunctionName; "
            "_collectNVVMFunctionNames",
        )
    if exact_shape.startswith("helper function parameter:"):
        if "Ptr<" in exact_shape:
            return (
                "helper-pointer-parameter-abi",
                "post-specialization linked IRFunc pointer signature; _validateNVVMHelperTarget",
            )
        return (
            "helper-aggregate-parameter-abi",
            "post-specialization linked IRFunc aggregate signature; _validateNVVMHelperTarget",
        )
    if exact_shape.startswith("helper function result type:"):
        if "Texture" in exact_shape or "Buffer" in exact_shape:
            return (
                "helper-resource-result-abi",
                "post-specialization linked IRFunc resource result; _validateNVVMHelperTarget",
            )
        return (
            "helper-aggregate-result-abi",
            "post-specialization linked IRFunc aggregate result; _validateNVVMHelperTarget",
        )
    if exact_shape.startswith("load result type: Array<"):
        return (
            "resource-array-value-load",
            "collectEntryPointUniforms -> synthesized GlobalParams resource-array field -> "
            "IRLoad; _validateNVVMFunction",
        )
    if exact_shape == "makeUInt64":
        return (
            "anyvalue-uint64-reconstruction",
            "AnyValue marshalling -> IRBuilder::emitMakeUInt64; _validateNVVMFunction",
        )
    if exact_shape == "makeArray":
        return (
            "fixed-array-value-construction",
            "legalizeMatrixTypes -> IRBuilder::emitMakeArray; _validateNVVMFunction",
        )
    if exact_shape.startswith("GenericAsm assembly="):
        return (
            "generic-asm-texture-query",
            "CUDA prelude texture dimensions -> StmtLoweringVisitor::visitIntrinsicAsmStmt -> "
            "IRGenericAsm; _validateNVVMFunction",
        )
    raise ValueError(f"discovery preflight shape has no producer audit: {exact_shape}")


def _failure_ownership(
    row: dict[str, str],
    exact_shape: str,
    selection_tags: str,
) -> tuple[str, str]:
    classification = row["classification"]
    if classification == "correct":
        return "", ""
    if classification == "preflight":
        return _preflight_cluster(exact_shape)
    if classification == "runtime-mismatch":
        return _runtime_cluster(selection_tags)
    if classification == "provider":
        if "global-to-generic UserPointer conversion" in exact_shape:
            return (
                "provider-global-user-pointer-cast",
                "_convertGlobalNVVMPointerToUserPointer -> "
                "NVVMIRBuilder::emitPointerAddressSpaceCast",
            )
        raise ValueError(f"provider shape has no producer audit: {exact_shape}")
    if classification == "infrastructure":
        if exact_shape == "target capability requirement":
            return (
                "infrastructure-cuda-capability",
                "entry-point target-requirement checking -> E36107",
            )
        if exact_shape == "NVRTC CUDA C++ compilation":
            return (
                "infrastructure-nvrtc-resource-compilation",
                "Slang CUDA C++ resource emission -> NVRTC 12.9",
            )
        return "infrastructure-other", "CUDA test infrastructure"
    raise ValueError(f"unknown discovery classification: {classification}")


def _build_pareto(
    mode_rows: list[dict[str, object]],
) -> list[dict[str, object]]:
    clusters: dict[str, dict[str, object]] = {}
    for row in mode_rows:
        cluster = str(row["cluster"])
        if not cluster:
            continue
        item = clusters.setdefault(
            cluster,
            {
                "cluster": cluster,
                "count": 0,
                "healthy_reference_count": 0,
                "classifications": set(),
                "exact_shapes": set(),
                "producer": row["producer"],
                "diagnostics": set(),
                "examples": [],
            },
        )
        item["count"] = int(item["count"]) + 1
        if row["native_reference_healthy"] == "true":
            item["healthy_reference_count"] = int(item["healthy_reference_count"]) + 1
        item["classifications"].add(row["classification"])
        item["exact_shapes"].add(row["exact_shape"])
        item["diagnostics"].add(row["diagnostic"])
        if len(item["examples"]) < 4:
            item["examples"].append(row["source"])

    result: list[dict[str, object]] = []
    for item in clusters.values():
        item["classifications"] = sorted(item["classifications"])
        item["exact_shapes"] = sorted(item["exact_shapes"])
        item["diagnostics"] = sorted(item["diagnostics"])
        result.append(item)
    return sorted(
        result,
        key=lambda item: (
            -int(item["healthy_reference_count"]),
            -int(item["count"]),
            item["cluster"],
        ),
    )


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--input", type=Path, default=Path("build/nvvm-discovery/results.tsv"))
    parser.add_argument(
        "--selection",
        type=Path,
        default=Path("build/nvvm-discovery/selected-workloads.tsv"),
    )
    parser.add_argument(
        "--frozen-v1-clusters",
        type=Path,
        default=Path("issue-nvvm-backend/census.slice-146-clusters.json"),
    )
    parser.add_argument(
        "--table",
        type=Path,
        default=Path("issue-nvvm-backend/discovery-census.slice-147.tsv"),
    )
    parser.add_argument(
        "--clusters",
        type=Path,
        default=Path("issue-nvvm-backend/discovery-census.slice-147-clusters.json"),
    )
    return parser.parse_args()


def main() -> int:
    args = parse_arguments()
    repo_root = args.repo.resolve()
    input_path = _resolve_path(repo_root, args.input)
    selection_path = _resolve_path(repo_root, args.selection)
    frozen_clusters_path = _resolve_path(repo_root, args.frozen_v1_clusters)
    table_path = _resolve_path(repo_root, args.table)
    clusters_path = _resolve_path(repo_root, args.clusters)

    raw_rows = _read_tsv(input_path)
    selections = {row["id"]: row for row in _read_tsv(selection_path)}
    by_id: dict[str, dict[str, dict[str, str]]] = {}
    for row in raw_rows:
        by_id.setdefault(row["id"], {})[row["mode"]] = row
    if set(by_id) != set(selections):
        raise SystemExit("raw discovery results do not match the selected manifest identities")

    output_rows: list[dict[str, object]] = []
    cluster_source_rows: dict[str, list[dict[str, object]]] = {mode: [] for mode in MODES}
    classification_totals: dict[str, dict[str, int]] = {mode: {} for mode in MODES}
    for workload_id in sorted(by_id):
        modes = by_id[workload_id]
        if set(modes) != set(MODES):
            raise SystemExit(f"incomplete mode results for {workload_id}: {sorted(modes)}")
        selection = selections[workload_id]
        native_healthy = modes["nvrtc-o3"]["classification"] == "correct"
        output_row: dict[str, object] = {
            "id": workload_id,
            "source": selection["source"],
            "selection_tags": selection["selection_tags"],
            "selection_rationale": selection["scope_reason"],
            "native_reference_healthy": "true" if native_healthy else "false",
        }
        for mode in MODES:
            raw = modes[mode]
            log_text = _read_log(repo_root, raw)
            exact_shape = _exact_shape(raw, log_text)
            diagnostic = _failure_diagnostic(raw, log_text)
            cluster, producer = _failure_ownership(
                raw,
                exact_shape,
                selection["selection_tags"],
            )
            classification = raw["classification"]
            classification_totals[mode][classification] = (
                classification_totals[mode].get(classification, 0) + 1
            )
            output_row[f"{mode}_classification"] = classification
            output_row[f"{mode}_elapsed_ms"] = raw["elapsed_ms"]
            output_row[f"{mode}_exact_shape"] = exact_shape
            output_row[f"{mode}_cluster"] = cluster
            output_row[f"{mode}_producer"] = producer
            output_row[f"{mode}_diagnostic"] = diagnostic
            output_row[f"{mode}_log"] = raw["log"]
            cluster_source_rows[mode].append(
                {
                    "source": selection["source"],
                    "classification": classification,
                    "exact_shape": exact_shape,
                    "cluster": cluster,
                    "producer": producer,
                    "diagnostic": diagnostic,
                    "native_reference_healthy": "true" if native_healthy else "false",
                }
            )
        output_rows.append(output_row)

    healthy_rows = [row for row in output_rows if row["native_reference_healthy"] == "true"]
    metrics = {
        "selected_workloads": len(output_rows),
        "healthy_reference_denominator": len(healthy_rows),
        "o0_correct": sum(row["nvvm-o0_classification"] == "correct" for row in healthy_rows),
        "o3_correct": sum(row["nvvm-o3_classification"] == "correct" for row in healthy_rows),
        "both_correct": sum(
            row["nvvm-o0_classification"] == "correct"
            and row["nvvm-o3_classification"] == "correct"
            for row in healthy_rows
        ),
    }
    pareto = {mode: _build_pareto(cluster_source_rows[mode]) for mode in MODES}
    frozen_clusters = json.loads(frozen_clusters_path.read_text(encoding="utf-8"))
    cluster_document = {
        "schema": 1,
        "corpus": "rolling discovery corpus",
        "metrics": metrics,
        "classification_totals": classification_totals,
        "pareto": pareto,
        "frozen_corpus_v1_slice_146_clusters": frozen_clusters,
    }
    _write_tsv(table_path, output_rows)
    clusters_path.parent.mkdir(parents=True, exist_ok=True)
    clusters_path.write_text(
        json.dumps(cluster_document, indent=2) + "\n",
        encoding="utf-8",
        newline="\n",
    )
    print(f"wrote {len(output_rows)} discovery rows to {table_path}")
    print(json.dumps(metrics, indent=2))
    print(json.dumps(classification_totals, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
