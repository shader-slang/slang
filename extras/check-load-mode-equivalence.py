#!/usr/bin/env python3
"""Compare slangc output across two settings of an environment toggle, over many shaders.

A development aid, not a CI job. It answers one question in bulk: does turning a load
mode on and off change what the compiler produces?

The in-suite check (`slang-unit-test-tool/irLoadEquivalence`) compiles a single shader
both ways on every build. This sweeps the whole test corpus instead, which is what finds
the divergences a single shader cannot -- an instruction shape that only appears in one
test, a decoration that only one module carries. It is slow by nature: two compiles per
shader, thousands of shaders.

Deliberately not tied to on-demand IR. Point it at any environment variable that selects
a load strategy and it will do the same comparison, which is the shape the on-demand
autodiff work needs too:

    # on-demand IR against eager
    extras/check-load-mode-equivalence.py --var SLANG_ONDEMAND_IR

    # a future autodiff toggle, over one directory
    extras/check-load-mode-equivalence.py --var SLANG_ONDEMAND_AUTODIFF tests/autodiff

What counts as a difference: exit status, stdout, or stderr. Shaders that fail
*identically* under both settings are reported as "both failed" and are not
divergences -- most of the corpus needs directives this script does not replicate, and
that is fine, since a shader the compiler rejects the same way twice tells us the two
paths agree.
"""

import argparse
import os
import subprocess
import sys
from pathlib import Path

# Emitted when the shader has no entry point of the requested name. Means this script's
# command line did not fit the shader, not that the compiler found anything wrong.
ENTRY_POINT_MISMATCH = b"E38000"


def compile_once(slangc, source, env_var, value, target, extra_args):
    env = dict(os.environ)
    env[env_var] = value
    # No -o: the generated code goes to stdout, which is what gets compared. Sending it
    # to /dev/null would leave only diagnostics, which agree in far more cases than the
    # code does and would make this look green while checking almost nothing.
    cmd = [str(slangc), str(source), "-target", target] + extra_args
    try:
        p = subprocess.run(cmd, env=env, capture_output=True, timeout=120)
        return p.returncode, p.stdout, p.stderr
    except subprocess.TimeoutExpired:
        return "timeout", b"", b""


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("roots", nargs="*", default=["tests"],
                    help="files or directories of .slang shaders (default: tests)")
    ap.add_argument("--slangc", default="build/Release/bin/slangc")
    ap.add_argument("--var", default="SLANG_ONDEMAND_IR",
                    help="environment variable to toggle (default: SLANG_ONDEMAND_IR)")
    ap.add_argument("--on", default="1", help="value for the feature-on run")
    ap.add_argument("--off", default="0", help="value for the feature-off run")
    ap.add_argument("--target", default="hlsl")
    ap.add_argument("--entry", default="computeMain")
    ap.add_argument("--limit", type=int, default=0, help="stop after N shaders (0 = all)")
    ap.add_argument("--compare", choices=["target", "ir"], default="target",
                    help="compare generated target code (default) or the IR itself. "
                         "'ir' is more sensitive: a body that lost children may never "
                         "reach codegen, so the emitted code can match while the IR does not")
    ap.add_argument("--quiet", action="store_true", help="only report divergences")
    args = ap.parse_args()

    slangc = Path(args.slangc)
    if not slangc.exists():
        sys.exit(f"slangc not found: {slangc}")

    # Preflight: confirm the compiler actually runs. Without this, anything that stops it
    # from starting -- a staged copy that cannot find its shared libraries, a half-written
    # binary from a concurrent build -- lands in the "failed the same way under both"
    # bucket and the run reports zero divergences, which reads as success while having
    # compiled nothing.
    probe = subprocess.run([str(slangc), "-v"], capture_output=True)
    if probe.returncode != 0:
        sys.exit(
            f"{slangc} did not run (exit {probe.returncode}). "
            f"stderr: {probe.stderr.decode(errors='replace').strip()[:300]}")

    sources = []
    for root in args.roots:
        p = Path(root)
        sources.extend(sorted(p.rglob("*.slang")) if p.is_dir() else [p])
    if args.limit:
        sources = sources[: args.limit]

    extra = ["-entry", args.entry, "-stage", "compute"]
    if args.compare == "ir":
        # -dump-ir writes the IR to stdout and needs -o to keep target code from mixing in.
        extra += ["-dump-ir", "-o", os.devnull]
    # A shared failure is not one thing. A shader the compiler parsed, checked and then
    # diagnosed is a real negative-test comparison: the two modes produced byte-identical
    # diagnostics. A shader rejected only because this script guessed the wrong entry
    # point never got that far -- it exercised session creation, builtin-module load and
    # entry-point lookup, and nothing else. Counting them together overstates the depth
    # of the coverage.
    diverged, agreed = [], 0
    negative_agreed = 0   # failed identically, having actually diagnosed something
    entry_mismatch = 0    # failed identically, but only because -entry did not match

    for i, src in enumerate(sources, 1):
        on = compile_once(slangc, src, args.var, args.on, args.target, extra)
        off = compile_once(slangc, src, args.var, args.off, args.target, extra)
        if on == off:
            if on[0] == 0:
                agreed += 1
            elif ENTRY_POINT_MISMATCH in on[2]:
                entry_mismatch += 1
            else:
                negative_agreed += 1
        else:
            diverged.append((src, on, off))
            print(f"DIVERGED: {src}")
            print(f"  {args.var}={args.on}: exit={on[0]}")
            print(f"  {args.var}={args.off}: exit={off[0]}")
            if on[1] != off[1]:
                print("  stdout differs")
            if on[2] != off[2]:
                print("  stderr differs")
        if not args.quiet and i % 100 == 0:
            print(f"... {i}/{len(sources)}  agreed={agreed} negative={negative_agreed} "
                  f"entry-mismatch={entry_mismatch} diverged={len(diverged)}",
                  file=sys.stderr)

    print(f"\n{len(sources)} shaders")
    print(f"  {agreed:5d}  compiled, generated output identical")
    print(f"  {negative_agreed:5d}  diagnosed, diagnostics identical (negative-test coverage)")
    print(f"  {entry_mismatch:5d}  skipped: no '{args.entry}' entry point, so only the load "
          f"path was compared")
    print(f"  {len(diverged):5d}  DIVERGED")
    if sources and agreed == 0:
        # Every shader was rejected, so the comparison only ever saw the error path.
        # Technically "no divergence", but it establishes nothing about generated code.
        print("WARNING: nothing compiled -- this run says nothing about codegen. The "
              "command line likely does not fit this corpus (wrong entry point, or "
              "shaders that are expected to fail).")
    return 1 if diverged else 0


if __name__ == "__main__":
    sys.exit(main())
