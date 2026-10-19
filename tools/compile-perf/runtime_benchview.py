"""Export archived runtime attempts through the shared BenchView host bundle."""
import argparse
import datetime
import hashlib
import json
from pathlib import Path
import re
import sys

import benchview

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "runtime-perf"))
import runtime_charts


def read(path):
    """Read an immutable measurement record without consulting the current machine."""
    return json.loads(path.read_text(encoding="utf-8"))


def observed_at(value):
    """Use the archived measurement time, never the import time or file timestamp."""
    return datetime.datetime.fromtimestamp(value, datetime.timezone.utc).isoformat()


def fingerprint(value):
    """Identify a measurement contract independently of the tested compiler revision."""
    return hashlib.sha256(benchview.encode(value)).hexdigest()


def observation(name, backend, implementation, measurement, contract, environment,
                source, observed, passed, metrics, reason=""):
    """Preserve failed observations and provenance without graphing invalid timings."""
    text = benchview.encode(source).decode()
    if len(text.encode()) > 262144:
        raise ValueError("Runtime source record exceeds the inline artifact limit")
    if len(metrics) > 64:
        raise ValueError("Runtime observation exceeds the metric limit")
    result = dict(
        test=dict(id="runtime-perf/" + name, name=name),
        case=dict(dimensions=dict(backend=backend, implementation=implementation,
                                  measurement=measurement, contract=fingerprint(contract))),
        observedAt=observed, status="passed" if passed else "failed",
        environment=dict(identity=dict(machine=environment.get("host", "unrecorded"),
                                        os=dict(name=environment.get("os", "unrecorded")))),
        metrics=metrics if passed else [],
        metadata=dict(measurementContract=contract, measuredEnvironment=environment),
        artifacts=[dict(id="source-record", name="Original runtime measurement",
                        fileName="record.json", mediaType="application/json",
                        content=dict(kind="inline-text", text=text))])
    workload = contract.get("workload", {})
    measurement_config = contract.get("envelope", {}).get("measurement", {})
    for key in ("elements", "batch", "size", "width"):
        value = workload.get(key, measurement_config.get(key))
        if value is not None:
            result["case"]["dimensions"][key] = value
    if not passed:
        result["error"] = dict(message=str(reason or "Runtime measurement failed or was unstable")[:4096])
    return result


def gpu_metric(stats):
    """Export the original per-execution GPU samples in milliseconds."""
    result = benchview.metric("gpu_iteration_ms", "GPU execution time", "ms", stats)
    if any(value < 0 for value in result["distribution"]["samples"]):
        raise ValueError("GPU timing samples must not be negative")
    result["id"] = "runtime.gpu_iteration_ms"
    return result


def observations(manifest, root, os_name):
    """Convert suite, paired compile and matched reference observations independently."""
    output = []
    started = observed_at(manifest["created_unix"])
    for backend, entry in manifest["backends"].items():
        if backend not in ("vulkan", "d3d12", "cuda"):
            raise ValueError("Unknown runtime backend")
        path = root / backend / "results.json"
        bundle = read(path) if path.exists() else None
        if bundle and bundle.get("schema_version") != 2:
            raise ValueError("Expected runtime schema 2")
        environment = bundle["environment"] if bundle else dict(os=os_name)
        if not environment.get("os", "").startswith(os_name):
            raise ValueError("Measured runtime OS disagrees with export OS")
        cases = {}
        for case in bundle["runs"] if bundle else []:
            if case["workload"] in cases or case.get("backend", backend) != backend:
                raise ValueError("Duplicate case or mismatched backend")
            cases[case["workload"]] = case
        if set(cases) - set(entry["expected_cases"]):
            raise ValueError("Unexpected runtime cases")
        for name in entry["expected_cases"]:
            case = cases.get(name, dict(workload=name, status="missing"))
            contract = runtime_charts.runtime_contract(dict(bundle=bundle, case=case)) if bundle else dict(backend=backend)
            passed = case.get("status") == "ok" and case.get("validated") is True
            source = {k: v for k, v in case.items() if k != "compile_runs"}
            metrics = [gpu_metric(case["timers"]["gpu_iteration_ms"])] if passed else []
            output.append(observation(name, backend, "slang", "suite", contract, environment,
                                      source, observed_at(bundle["created_unix"]) if bundle else started,
                                      passed, metrics, case.get("error") or case.get("status")))
            # Compilation is independently sampled and may succeed even if execution fails.
            for compile_run in case.get("compile_runs", []):
                item = benchview.observation(compile_run, started, environment.get("host", "unrecorded"), os_name)
                item["test"] = dict(id="runtime-perf/" + name, name=name)
                item["case"]["dimensions"].update(backend=backend, implementation="slang",
                    measurement="compilation", stage=compile_run["workload"], contract=fingerprint(contract))
                item["metadata"].update(measurementContract=contract, measuredEnvironment=environment)
                output.append(item)
    reference_path = root / "references/results.json"
    if reference_path.exists():
        data = read(reference_path)
        if not data["environment"]["os"].startswith(os_name):
            raise ValueError("Measured reference OS disagrees with export OS")
        for row in data["cases"]:
            if row["backend"] not in manifest["backends"]:
                raise ValueError("Unexpected reference backend")
            if row["case"] not in manifest["backends"][row["backend"]]["expected_cases"]:
                raise ValueError("Unexpected reference case")
            for runs in row["runs"].values():
                for run in runs:
                    if (run["elements"] != data["measurement"]["elements"] or
                            run["batch"] != row["batch"] or run["backend"] != row["backend"]):
                        raise ValueError("Reference samples have mismatched dimensions")
            contract = runtime_charts.reference_contract(dict(references=data, reference=row))
            for implementation in ("slang", "reference"):
                runs = row["runs"][implementation]
                passed = (data.get("complete") is True and row["summary"][implementation]["valid"] is True
                          and bool(runs) and all(r.get("status") == "ok" and r.get("validated") is True for r in runs))
                stats = dict(samples=[s["gpu_iteration_ms"] for r in runs for s in r["samples"]]) if passed else None
                source = dict(runs=runs, summary=row["summary"][implementation],
                              reference_compilation=row.get("reference_compilation"))
                output.append(observation(row["case"], row["backend"], implementation, "matched-reference",
                    contract, data["environment"], source, observed_at(data["created_unix"]),
                    passed, [gpu_metric(stats)] if passed else []))
    elif "references" in manifest:
        output.append(observation("reference-collection", "all", "reference", "collection", {},
            dict(os=os_name), manifest["references"], started, False, [], "Reference results unavailable"))
    if not output:
        output.append(observation("collection", "all", "slang", "collection", {}, dict(os=os_name),
                                  manifest, started, False, [], manifest.get("reason")))
    identities = [(item["test"]["id"], benchview.encode(item["case"])) for item in output]
    if len(set(identities)) != len(identities):
        raise ValueError("Duplicate runtime observation")
    return output


def submissions(path, os_name, branch=None):
    """Build runtime batches with daily/release identity and stable replay semantics."""
    manifest = read(path)
    if manifest.get("schema_version") != 1 or manifest.get("status") == "running":
        raise ValueError("Expected a finalized runtime manifest")
    revision = manifest["commit"]
    if not re.fullmatch(r"[0-9a-fA-F]{40}", revision):
        raise ValueError("A full tested compiler commit SHA is required")
    kind = manifest.get("kind", "daily")
    if kind not in ("daily", "release"):
        raise ValueError("Unknown runtime cadence")
    dimensions = dict(kind=kind)
    run = dict(key="git:" + revision + "/suite:runtime-perf/config:" + fingerprint(dimensions),
               suite=dict(id="runtime-perf", name="Runtime performance"),
               vcs=dict(repository="https://github.com/shader-slang/slang", revision=revision),
               dimensions=dimensions)
    if branch:
        run["vcs"]["branch"] = branch
    if kind == "release":
        if not re.fullmatch(r"v\d{4}\.\d+", manifest["label"]):
            raise ValueError("Expected a main release label")
        run["metadata"] = dict(release=dict(label=manifest["label"]))
    envelope = dict(schemaVersion=1, project=dict(id="slang", name="Slang"),
                    producer=dict(name="slang-runtime-perf", version="1"), run=run,
                    execution=dict(id=manifest["label"] + "/" + manifest["attempt"], metadata=manifest))
    return benchview.batch_submissions(envelope, observations(manifest, path.parent, os_name))


def main():
    """Support replay and main-release imports through the same host export format."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", required=True, type=Path)
    parser.add_argument("--os", required=True)
    parser.add_argument("--branch")
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    (args.output / "submissions.ready").unlink(missing_ok=True)
    payloads = submissions(args.manifest, args.os, args.branch)
    benchview.export(payloads, args.output)
    print(f"Exported {sum(len(p['observations']) for p in payloads)} runtime observations")


if __name__ == "__main__":
    try:
        main()
    except (ValueError, KeyError, TypeError, AttributeError, RuntimeError, OSError) as error:
        print(f"Runtime BenchView export failed: {error}", file=sys.stderr)
        sys.exit(1)
