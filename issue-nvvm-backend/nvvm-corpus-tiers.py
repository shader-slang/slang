#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Select runtime corpus configurations without owning or rewriting acceptance results."""
import hashlib
import importlib.util
import json
import re
from pathlib import Path

MODES = ("nvrtc-o3", "nvvm-o0", "nvvm-o3")
CORRECT_COUNTS = dict(passed=1, executed=1, ignored=0, other_summary_status=0)


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def read(path):
    return json.loads(Path(path).read_text())


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def at(value, path):
    for key in path:
        value = value[key]
    return value


def correct(row):
    return (row.get("classification") == "correct" and row.get("return_code") == 0
            and row.get("execution_counts") == CORRECT_COUNTS)


def workload_inventory(repo):
    """Keep historical IDs; add eligible authored CUDA cases once per source/ordinal."""
    here = repo / "issue-nvvm-backend"
    census = load("tier_census", here / "run-compute-census.py")
    discovery = load("tier_discovery", here / "run-compute-discovery.py")
    native, _ = census.discover_workloads(repo / "tests", 80)
    frozen = census.select_frozen_workloads(native, here / "census.slice-195.tsv")
    sources, _ = discovery._audit_frozen_v1(here / "census.slice-146.tsv")
    discovered, _ = discovery._load_discovery_workloads(
        repo / "tests", here / "discovery-corpus.manifest.tsv", sources, census)
    rows, identities, ids = [], set(), set()
    for origin, group in (("frozen", frozen), ("discovery", discovered), ("candidate", native)):
        for row in group:
            identity = (row["source"], row["source_test_ordinal"])
            if origin == "candidate" and identity in identities:
                continue
            if identity in identities or row["id"] in ids:
                raise ValueError("duplicate corpus identity: " + str(identity))
            identities.add(identity)
            ids.add(row["id"])
            rows.append(dict(row, origin=origin))
    census.select_architecture(rows, 80)
    return rows


def oracle_paths(repo, row):
    """Match the existing runner's selected expected-file fallback exactly."""
    source = repo / "tests" / row["source"]
    ordinal = row["source_test_ordinal"]
    suffix = f".{ordinal}.expected.txt" if ordinal else ".expected.txt"
    expected = Path(str(source) + suffix)
    if not expected.is_file() and ordinal:
        expected = Path(str(source) + ".expected.txt")
    return [expected] if expected.is_file() else []


def build_inventory(repo, baseline, focused, metadata, workloads=None):
    workloads = workload_inventory(repo) if workloads is None else workloads
    known = {row["id"]: row for row in workloads}
    if len(known) != len(workloads):
        raise ValueError("duplicate workload ID")
    accepted = {}
    for corpus in baseline["corpora"].values():
        for row in corpus["fresh_cell_outcomes"]:
            key = (row["id"], row["mode"])
            if key in accepted or row["id"] not in known or row["mode"] not in MODES:
                raise ValueError("invalid or missing accepted cell: " + str(key))
            if row.get("classification") == "correct" and not correct(row):
                raise ValueError("accepted correct cell lacks exact execution counts")
            accepted[key] = row
    admissions = {}
    for item in metadata["admissions"]:
        key = (item["id"], item["mode"])
        if key in admissions or item["id"] not in known or item["mode"] not in MODES:
            raise ValueError("invalid or duplicate focused admission: " + str(key))
        feature = focused["features"][item["feature"]]
        if feature["status"] != "accepted-focused":
            raise ValueError("admission requires accepted focused evidence")
        outcome = at(feature, item["outcome_path"])
        if isinstance(outcome, dict):
            if not correct(outcome) or (outcome["id"], outcome["mode"]) != key:
                raise ValueError("focused outcome does not prove this exact cell")
        else:
            name = "tests/" + known[item["id"]]["source"]
            match = re.fullmatch(re.escape(name) + r"(?:\.(\d+))? \(cuda\)", str(item["outcome_path"][-1]))
            if outcome != "passed" or not match:
                raise ValueError("focused admission is not this source's passed runtime outcome")
            census = load("admission_census", repo / "issue-nvvm-backend/run-compute-census.py")
            ordinal = int(match[1] or 0)
            directives = census.enumerate_test_directives((repo / name).read_text())
            directive = next((row for row in directives if row["test_ordinal"] == ordinal), None)
            if directive is None or not census.is_active_compare_directive(directive):
                raise ValueError("admitted runtime directive is missing")
            arguments = directive["arguments"]
            optimization = re.findall(r"(?:^|\s)-O([03])(?:\s|$)", arguments)
            authored_mode = ("nvvm" if "emit-cuda-via-nvvm" in arguments else "nvrtc") + "-o" + (optimization[-1] if optimization else "3")
            if authored_mode != item["mode"]:
                raise ValueError("focused runtime outcome contradicts admitted mode")
        # The metadata binds one reviewed mode/oracle mapping, not an entire feature status.
        digest = at(feature, item["source_hash_path"])
        source = "tests/" + known[item["id"]]["source"]
        hash_owner = at(feature, item["source_hash_path"][:-1])
        if item["source_hash_path"][-1] != source and hash_owner.get("path") != source:
            raise ValueError("focused source identity belongs to a different input")
        if not isinstance(digest, str) or len(digest) != 64:
            raise ValueError("admission lacks exact source identity")
        admissions[key] = (item, digest)
    smoke = set()
    for item in metadata["smoke"]:
        key = (item["id"], item["mode"])
        if key in smoke or item["id"] not in known or item["mode"] not in MODES:
            raise ValueError("invalid or duplicate smoke cell: " + str(key))
        smoke.add(key)
    priorities = metadata.get("exploratory_priority", {})
    if set(priorities) - set(known):
        raise ValueError("unknown exploratory priority ID")
    cells = []
    for workload in workloads:
        name = "tests/" + workload["source"]
        actual = sha(repo / name)
        for mode in MODES:
            key = (workload["id"], mode)
            before = accepted.get(key)
            admission = admissions.get(key)
            working = bool(admission or before and correct(before))
            expected = baseline.get("runtime_input_sha256", {}).get(name)
            evidence = "accepted-baseline" if before else None
            if admission:
                item, expected = admission
                evidence = "focused-evidence/features/" + item["feature"]
            changes = []
            if working and expected != actual:
                changes.append(dict(path=name, accepted_sha256=expected, current_sha256=actual))
            inputs = {name: actual}
            ordinal = workload["source_test_ordinal"]
            expected_names = [name + (f".{ordinal}.expected.txt" if ordinal else ".expected.txt")]
            if ordinal:
                expected_names.append(name + ".expected.txt")
            accepted_oracle = next((p for p in expected_names if p in metadata.get("oracle_inputs", {})), None)
            current_oracles = oracle_paths(repo, workload)
            if working and accepted_oracle and not (repo / accepted_oracle).is_file():
                changes.append(dict(path=accepted_oracle,
                                    accepted_sha256=metadata["oracle_inputs"][accepted_oracle],
                                    current_sha256=None))
            for path in current_oracles:
                relative = path.relative_to(repo).as_posix()
                digest = sha(path)
                inputs[relative] = digest
                prior = metadata.get("oracle_inputs", {}).get(relative)
                if working and prior != digest:
                    changes.append(dict(path=relative, accepted_sha256=prior, current_sha256=digest))
            if key in smoke and not working:
                raise ValueError("smoke must be a subset of working: " + str(key))
            cells.append(dict(id=workload["id"], source=workload["source"],
                              source_test_ordinal=workload["source_test_ordinal"], mode=mode,
                              minimum_architecture=80, capability=workload.get("capability", "cuda_sm_8_0"),
                              validation="runtime", origin=workload["origin"],
                              tier="working" if working else "exploratory", smoke=key in smoke,
                              evidence=evidence, accepted_outcome=before,
                              input_sha256=inputs, input_changes=changes,
                              reason="accepted" if working else "known-failure" if before else "untested",
                              priority=priorities.get(workload["id"], {}).get("rank", 100),
                              priority_reason=priorities.get(workload["id"], {}).get("reason", ""),
                              failure_category=priorities.get(workload["id"], {}).get("failure_category", "unclassified")))
    return cells


def select(cells, tier, modes=None, limit=None):
    if modes and (len(set(modes)) != len(modes) or set(modes) - set(MODES)):
        raise ValueError("invalid or duplicate mode selection")
    if limit is not None and (tier != "exploratory" or limit < 1):
        raise ValueError("a positive limit is only supported for exploratory batches")
    selected = [row for row in cells if (row["smoke"] if tier == "smoke" else row["tier"] == tier)
                and (not modes or row["mode"] in modes)]
    if tier == "exploratory":
        selected.sort(key=lambda row: (row["reason"] != "known-failure", row["priority"],
                                      row["id"], row["mode"]))
    if limit:
        selected = selected[:limit]
    if not selected:
        raise ValueError("empty tier selection")
    return selected
