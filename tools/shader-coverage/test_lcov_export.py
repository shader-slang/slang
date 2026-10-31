#!/usr/bin/env python3
"""Run statement coverage through strict LCOV consumers without requiring a GPU.

Run with --slangc BUILD/bin/slangc --output-dir DIR. Requires a C++ compiler
and genhtml. Preserves source-entry counts; no gcov line-frequency equivalence
is assumed. The output directory keeps generated code, manifests, counters,
LCOV, and HTML for all/line/branch/function x count/boolean x 32/64-bit runs.
"""
import argparse
import json
from pathlib import Path
import re
import struct
import subprocess
import sys


def run(argv, cwd):
    process = subprocess.run(
        list(map(str, argv)), cwd=cwd, text=True, capture_output=True
    )
    if process.returncode:
        raise RuntimeError(
            f"{' '.join(map(str, argv))}\n{process.stdout}\n{process.stderr}"
        )
    return process.stdout


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--slangc", required=True)
    parser.add_argument("--cxx", default="clang++")
    parser.add_argument("--output-dir", required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    out = Path(args.output_dir).resolve()
    out.mkdir(parents=True, exist_ok=True)
    source = root / "tests/language-feature/coverage/lcov/semantics.slang"
    tags = {
        match[1]: line_number
        for line_number, line in enumerate(source.read_text().splitlines(), 1)
        if (match := re.search(r"CASE: (\w+)", line))
    }
    reference = out / "reference.cpp"
    reference.write_text(
        "#include <cstdio>\n#include <initializer_list>\n#define CPU_REFERENCE\n#include "
        + json.dumps(str(source))
        + "\nint main() { for(int x : {-1, 0, 1, 2}) "
        + 'printf("%d ", sequential(x)+sameLine(x)+disjoint(x)+parenthesized(x)'
        + "+choose(x)+logical(x)+mixed(x)+loop(x)+cases(x)+whileLoop(x)); }\n"
    )
    run([args.cxx, "-std=c++17", "-O0", reference, "-o", out / "reference"], out)
    expected_output = run([out / "reference"], out).split()
    results = []
    for kinds in ["all", "line", "branch", "function"]:
        for mode in ["count", "boolean"]:
            for width in [32, 64]:
                name = f"{kinds}-{mode}-{width}"
                cpp = out / (name + ".cpp")
                flags = ["-trace-coverage-counter-width", str(width), "-validate-ir"]
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
                manifest_path = Path(str(cpp) + ".coverage-manifest.json")
                manifest = json.loads(manifest_path.read_text())
                count = manifest["counter_count"]
                offset = manifest["buffer"]["uniform_offset"]
                driver = out / (name + "-driver.cpp")
                driver.write_text(
                    f"""#include {json.dumps(str(cpp))}
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
                )
                run([args.cxx, "-std=c++17", "-O0", driver, "-o", out / "shader"], out)
                actual_output = run([out / "shader"], out).split()
                assert actual_output == expected_output, (
                    name,
                    actual_output,
                    expected_output,
                )
                raw = (out / (name + ".bin")).read_bytes()
                counters = struct.unpack(
                    "<" + ("I" if width == 32 else "Q") * count, raw
                )
                lcov = out / (name + ".lcov")
                run(
                    [
                        sys.executable,
                        root / "tools/shader-coverage/slang-coverage-to-lcov.py",
                        "--manifest",
                        manifest_path,
                        "--counters",
                        out / (name + ".bin"),
                        "--output",
                        lcov,
                    ],
                    out,
                )
                lines, branches = {}, {}
                for record in lcov.read_text().splitlines():
                    if record.startswith("DA:"):
                        line, hits = record[3:].split(",")[:2]
                        lines[int(line)] = int(hits)
                    elif record.startswith("BRDA:"):
                        line, site, arm, hits = record[5:].split(",")
                        branches.setdefault(int(line), []).append(hits)
                if kinds in ("all", "line"):
                    # These are deliberately statement-event aggregates, not
                    # gcov line visits (12 vs 4, 8 vs 4, and 4 vs 7).
                    for tag, hits in [
                        ("sequential", 12),
                        ("same_line", 8),
                        ("disjoint", 4),
                        ("while_header", 4),
                    ]:
                        expected = 1 if mode == "boolean" else hits
                        assert lines[tags[tag]] == expected, (name, tag, lines)
                    # Keep individual arm frequencies in the raw metadata.
                    arms = [
                        counters[entry["counter"]]
                        for entry in manifest["entries"]
                        if entry["kind"] == "line" and entry["line"] == tags["disjoint"]
                    ]
                    assert arms == ([1, 1] if mode == "boolean" else [2, 2]), (
                        name,
                        arms,
                    )
                if kinds in ("all", "branch"):
                    assert sorted(branches[tags["outer_unreached"]]) == (
                        ["0", "1"] if mode == "boolean" else ["0", "4"]
                    ), name
                    assert branches[tags["never_evaluated"]] == ["-", "-"], name
                if mode == "boolean":
                    assert all(value in (0, 1) for value in lines.values()), name
                html_log = run(
                    [
                        "genhtml",
                        lcov,
                        "--branch-coverage",
                        "--output-directory",
                        out / (name + "-html"),
                    ],
                    out,
                )
                (out / (name + "-genhtml.log")).write_text(html_log)
                results.append({"name": name, "outputs": actual_output, "lines": lines})
    (out / "results.json").write_text(json.dumps(results, indent=2) + "\n")
    print(
        "PASS: 16 LCOV runs; statement aggregates and shader outputs preserved; strict genhtml"
    )


if __name__ == "__main__":
    main()
