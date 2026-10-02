#!/usr/bin/env python3
"""Execute the same scalar fixture with GNU gcov and Slang coverage.

Run with --slangc BUILD/bin/slangc --gcc g++-15 --gcov gcov-15.
The output directory preserves the pinned reference, manifests, raw counts,
LCOV, and strict genhtml reports. No GPU is needed for this semantic gate.
"""
import argparse
import gzip
import json
from pathlib import Path
import re
import subprocess
import sys


def run(argv, cwd):
    p = subprocess.run(list(map(str, argv)), cwd=cwd, text=True, capture_output=True)
    if p.returncode:
        raise RuntimeError(f"{' '.join(map(str, argv))}\n{p.stdout}\n{p.stderr}")
    return p.stdout or p.stderr


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--slangc", required=True)
    p.add_argument("--gcc", default="g++-15")
    p.add_argument("--gcov", default="gcov-15")
    p.add_argument("--cxx", default="clang++")
    p.add_argument("--output-dir", required=True)
    args = p.parse_args()
    root = Path(__file__).resolve().parents[2]
    out = Path(args.output_dir).resolve()
    out.mkdir(parents=True, exist_ok=True)
    source = root / "tests/language-feature/coverage/gcov/semantics.slang"
    tags = {
        m[1]: i
        for i, line in enumerate(source.read_text().splitlines(), 1)
        if (m := re.search(r"CASE: (\w+)", line))
    }
    body = (
        "int inputs[] = {-1,0,1,2}; for(int x : inputs) "
        'printf("%d ", sequential(x)+sameLine(x)+choose(x)+logical(x)'
        "+mixed(x)+loop(x)+cases(x));"
    )
    (out / "reference.cpp").write_text(
        "#include <cstdio>\n#define CPU_REFERENCE\n#include "
        + json.dumps(str(source))
        + "\nint main(){"
        + body
        + "}\n"
    )
    for f in out.glob("*.gcda"):
        f.unlink()
    run(
        [args.gcc, "-O0", "--coverage", "-c", "reference.cpp", "-o", "reference.o"], out
    )
    run([args.gcc, "--coverage", "reference.o", "-o", "reference"], out)
    expected_output = run([out / "reference"], out).split()
    run([args.gcov, "-b", "-c", "-j", "reference.gcno"], out)
    reference = json.loads(
        gzip.decompress((out / "reference.gcov.json.gz").read_bytes())
    )
    ref_file = next(
        f for f in reference["files"] if f["file"].endswith("semantics.slang")
    )
    ref_lines = {x["line_number"]: x for x in ref_file["lines"]}
    results = {
        "gcc": run([args.gcc, "--version"], out).splitlines()[0],
        "slang": run([Path(args.slangc).resolve(), "-version"], out).strip(),
        "cases": {tag: ref_lines.get(line) for tag, line in tags.items()},
        "runs": [],
    }
    failures = []
    for kinds in ["all", "line", "branch", "function"]:
        for mode in ["count", "boolean"]:
            for width in [32, 64]:
                name = f"{kinds}-{mode}-{width}"
                cpp = out / (name + ".cpp")
                flags = ["-trace-coverage-counter-width", str(width)]
                for kind, flag in [
                    ("line", "-trace-coverage"),
                    ("function", "-trace-function-coverage"),
                    ("branch", "-trace-branch-coverage"),
                ]:
                    if kinds in ("all", kind):
                        flags.append(flag)
                if mode == "boolean":
                    flags.append("-trace-coverage-boolean")
                run(
                    [
                        Path(args.slangc).resolve(),
                        source,
                        "-entry",
                        "computeMain",
                        "-target",
                        "cpp",
                        *flags,
                        "-o",
                        cpp,
                    ],
                    out,
                )
                manifest = json.loads(
                    Path(str(cpp) + ".coverage-manifest.json").read_text()
                )
                count = manifest["counter_count"]
                offset = manifest["buffer"]["uniform_offset"]
                driver = f"""#include {json.dumps(str(cpp))}
    #include <cstdio>
    #include <cstring>
    struct BufferView {{ void* data; size_t count; }};
    int main() {{
        int inputs[] = {{-1,0,1,2}}, outputs[4] = {{}};
        uint{width}_t counters[{count}] = {{}};
        alignas(16) unsigned char params[{offset}+sizeof(BufferView)] = {{}};
        BufferView in = {{inputs,4}}, output = {{outputs,4}}, cov = {{counters,{count}}};
        memcpy(params,&in,sizeof(in)); memcpy(params+sizeof(in),&output,sizeof(output));
        memcpy(params+{offset},&cov,sizeof(cov));
        ComputeVaryingInput varying = {{}}; varying.endGroupID = {{4,1,1}};
        computeMain(&varying,nullptr,params);
        FILE* f = fopen("{name}.bin","wb"); if(!f) return 2;
        fwrite(counters,sizeof(counters),1,f); fclose(f);
        for(int x : outputs) printf("%d ",x);
    }}
    """
                (out / "driver.cpp").write_text(driver)
                run([args.cxx, "-std=c++17", "-O0", "driver.cpp", "-o", "shader"], out)
                actual_output = run([out / "shader"], out).split()
                assert actual_output == expected_output, (
                    name,
                    actual_output,
                    expected_output,
                )
                run(
                    [
                        sys.executable,
                        root / "tools/shader-coverage/slang-coverage-to-lcov.py",
                        "--manifest",
                        str(cpp) + ".coverage-manifest.json",
                        "--counters",
                        out / (name + ".bin"),
                        "--output",
                        out / (name + ".lcov"),
                    ],
                    out,
                )
                lines = {}
                branches = {}
                for record in (out / (name + ".lcov")).read_text().splitlines():
                    if record.startswith("DA:"):
                        line, hits = record[3:].split(",")[:2]
                        lines[int(line)] = int(hits)
                    elif record.startswith("BRDA:"):
                        line, site, arm, hits = record[5:].split(",")
                        branches.setdefault(int(line), []).append(
                            None if hits == "-" else int(hits)
                        )
                # GCC associates shared condition/merge blocks with ternary arm
                # lines even at -O0. Preserve precise source-arm executions instead
                # of reproducing that backend-specific line mapping.
                for tag in (
                    ["true_arm", "false_arm"]
                    if kinds in ("all", "line", "branch")
                    else []
                ):
                    want = 1 if mode == "boolean" else 2
                    if lines.get(tags[tag]) != want:
                        failures.append(
                            f"{name}: {tag}: expected {want} arm executions"
                        )
                for tag in (
                    ["sequential", "same_line", "loop_line"]
                    if kinds in ("all", "line")
                    else []
                ):
                    line = tags[tag]
                    want = ref_lines[line]["count"]
                    if mode == "boolean":
                        want = int(want != 0)
                    if lines.get(line) != want:
                        failures.append(
                            f"{name}: {tag}: {lines.get(line)} != gcov {want}"
                        )
                if kinds in ("all", "branch"):
                    rhs = branches.get(tags["skipped_rhs"], [])
                    if sorted(-1 if x is None else x for x in rhs) != [-1, -1, 0, 1]:
                        failures.append(f"{name}: skipped RHS decisions: {rhs}")
                    for tag in ["same_line", "mixed", "switch", "loop_line"]:
                        ref = ref_lines[tags[tag]]["branches"]
                        want = sorted(
                            int(b["count"] != 0) if mode == "boolean" else b["count"]
                            for b in ref
                        )
                        got = sorted(
                            -1 if x is None else x for x in branches.get(tags[tag], [])
                        )
                        if got != want:
                            failures.append(
                                f"{name}: {tag} outcomes: {got} != gcov {want}"
                            )
                run(
                    [
                        "genhtml",
                        out / (name + ".lcov"),
                        "--branch-coverage",
                        "--output-directory",
                        out / (name + "-html"),
                    ],
                    out,
                )
                results["runs"].append(
                    {"name": name, "lines": lines, "branches": branches}
                )
    results["failures"] = failures
    (out / "results.json").write_text(json.dumps(results, indent=2) + "\n")
    if failures:
        raise AssertionError("\n".join(failures))
    print(
        f'PASS: GCC {reference["gcc_version"]}; sixteen all/line/branch/function × count/boolean × 32/64-bit runs; strict genhtml'
    )


if __name__ == "__main__":
    main()
