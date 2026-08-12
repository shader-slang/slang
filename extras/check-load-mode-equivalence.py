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


def compile_once(slangc, source, env_var, value, target, extra_args):
    env = dict(os.environ)
    env[env_var] = value
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
    ap.add_argument("--quiet", action="store_true", help="only report divergences")
    args = ap.parse_args()

    slangc = Path(args.slangc)
    if not slangc.exists():
        sys.exit(f"slangc not found: {slangc}")

    sources = []
    for root in args.roots:
        p = Path(root)
        sources.extend(sorted(p.rglob("*.slang")) if p.is_dir() else [p])
    if args.limit:
        sources = sources[: args.limit]

    extra = ["-entry", args.entry, "-stage", "compute", "-o", os.devnull]
    diverged, both_failed, agreed = [], 0, 0

    for i, src in enumerate(sources, 1):
        on = compile_once(slangc, src, args.var, args.on, args.target, extra)
        off = compile_once(slangc, src, args.var, args.off, args.target, extra)
        if on == off:
            if on[0] == 0:
                agreed += 1
            else:
                both_failed += 1
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
            print(f"... {i}/{len(sources)}  agreed={agreed} both-failed={both_failed} "
                  f"diverged={len(diverged)}", file=sys.stderr)

    print(f"\n{len(sources)} shaders: {agreed} agreed, {both_failed} failed the same way "
          f"under both, {len(diverged)} diverged")
    return 1 if diverged else 0


if __name__ == "__main__":
    sys.exit(main())
