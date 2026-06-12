#!/usr/bin/env python3
"""Local branch-vs-branch compile-time comparison — the dev-side equivalent of
the per-PR gate (see DESIGN.md; the automated workflow is a later phase).

Builds slangc from a baseline ref (default: master) and from your current working
tree, benchmarks both on THIS machine back-to-back, and runs compare.py — so you
can answer "did my branch slow compilation?" before pushing. Building both on one
box cancels machine variance, leaving your change's actual impact.

    python compare_branches.py                          # current tree vs master, release
    python compare_branches.py --base nv-master --only minimal,autodiff --samples 3
    python compare_branches.py --config debug           # fast functional smoke (noisy timings)
    python compare_branches.py --base-slangc <path>     # reuse a prebuilt baseline, skip its build

Cross-platform (Windows / Linux / macOS): no shell, slangc[.exe] is located by
search (config dir varies by platform), all paths via os.path.join. The baseline
is built in a throwaway `git worktree`, so your checkout is never touched.

Notes:
- The build is the slow part (minutes per side). Use --only / --samples for quick
  checks. --config debug is fast but timings are noisy — use release for numbers
  you'd act on.
- Windows: the default `default` (Ninja) preset needs a compiler in the
  environment (run from a VS Developer prompt). VS users can instead pass
  --configure-preset vs2022 --build-preset vs2022-release.
- mdl_dxr needs the corpus first (python fetch_corpus.py --name mdl) or drop it
  from --only.
- Exit code is compare.py's: non-zero if a workload regressed past --threshold,
  so this doubles as a pre-push check.
"""
import argparse
import os
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
EXE = ".exe" if os.name == "nt" else ""
PR_SUBSET = ("minimal,autodiff,dynamic_dispatch,diagnostics_errors,"
             "diagnostics_clean,specialization,inlining,mdl_dxr")


def run(cmd, cwd=None, check=True):
    where = f"  (cwd={cwd})" if cwd else ""
    print(f"$ {subprocess.list2cmdline(cmd)}{where}", flush=True)
    rc = subprocess.run(cmd, cwd=cwd).returncode
    if check and rc != 0:
        sys.exit(f"command failed (exit {rc}): {subprocess.list2cmdline(cmd)}")
    return rc


def git_out(args, cwd):
    return subprocess.run(["git", *args], cwd=cwd, capture_output=True,
                          encoding="utf-8", errors="replace").stdout.strip()


def find_slangc(build_root):
    """Locate slangc[.exe] under a build tree; the config subdir (Release/Debug/…)
    varies by platform and preset, so search rather than hard-code the path."""
    target = "slangc" + EXE
    best = None
    for dirpath, _, files in os.walk(build_root):
        if os.path.basename(dirpath) == "bin" and target in files:
            cand = os.path.join(dirpath, target)
            if best is None or os.path.getmtime(cand) > os.path.getmtime(best):
                best = cand
    return best


def build_slangc(checkout, configure_preset, build_preset):
    run(["cmake", "--preset", configure_preset], cwd=checkout)
    run(["cmake", "--build", "--preset", build_preset, "--target", "slangc"], cwd=checkout)
    slangc = find_slangc(os.path.join(checkout, "build"))
    if not slangc:
        sys.exit(f"build succeeded but slangc{EXE} not found under {checkout}{os.sep}build")
    return slangc


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--base", default="master", help="baseline git ref (default: master)")
    ap.add_argument("--config", choices=["release", "debug"], default="release",
                    help="build config for BOTH sides (default: release)")
    ap.add_argument("--configure-preset", default="default",
                    help="cmake configure preset (default: default; VS users: vs2022)")
    ap.add_argument("--build-preset", default=None,
                    help="cmake build preset (default: matches --config, e.g. 'release')")
    ap.add_argument("--only", default=PR_SUBSET, help="comma-separated workload subset")
    ap.add_argument("--samples", type=int, default=5)
    ap.add_argument("--warmup", type=int, default=1)
    ap.add_argument("--threshold", type=float, default=15.0,
                    help="percent change compare.py flags (default 15)")
    ap.add_argument("--base-slangc", default=None,
                    help="prebuilt baseline slangc[.exe] (skip building the base)")
    ap.add_argument("--head-slangc", default=None,
                    help="prebuilt head slangc[.exe] (skip building the current tree)")
    ap.add_argument("--repo", default=None, help="slang checkout root (default: auto)")
    args = ap.parse_args()

    build_preset = args.build_preset or args.config
    repo = args.repo or git_out(["rev-parse", "--show-toplevel"], HERE) \
        or os.path.abspath(os.path.join(HERE, "..", "..", ".."))
    py = sys.executable
    results_dir = os.path.join(HERE, "results")

    head_branch = git_out(["rev-parse", "--abbrev-ref", "HEAD"], repo) or "HEAD"
    base_sha = git_out(["rev-parse", "--short", args.base], repo)
    if not base_sha:
        sys.exit(f"base ref '{args.base}' not found in {repo}")
    print(f"== compile-perf: {head_branch} (head) vs {args.base}@{base_sha} (base), "
          f"config={args.config} ==\n")

    # HEAD = the current working tree.
    head_slangc = args.head_slangc or build_slangc(repo, args.configure_preset, build_preset)

    # BASE = a throwaway worktree at the baseline ref (your checkout stays untouched).
    tmp_parent = None
    if args.base_slangc:
        base_slangc = args.base_slangc
    else:
        tmp_parent = tempfile.mkdtemp(prefix="slang-perf-")
        worktree = os.path.join(tmp_parent, "base")  # git worktree add creates it
        try:
            run(["git", "worktree", "add", "--detach", worktree, args.base], cwd=repo)
            run(["git", "submodule", "update", "--init", "--recursive"], cwd=worktree)
            base_slangc = build_slangc(worktree, args.configure_preset, build_preset)
        except BaseException:
            subprocess.run(["git", "worktree", "remove", "--force", worktree], cwd=repo)
            shutil.rmtree(tmp_parent, ignore_errors=True)
            raise

    try:
        for label, slangc in (("base", base_slangc), ("head", head_slangc)):
            run([py, os.path.join(HERE, "bench.py"), "--slangc", slangc, "--label", label,
                 "--out", results_dir, "--only", args.only,
                 "--samples", str(args.samples), "--warmup", str(args.warmup)])
        rc = run([py, os.path.join(HERE, "compare.py"), "base", "head",
                  "--results", results_dir, "--threshold", str(args.threshold)], check=False)
    finally:
        if tmp_parent:
            subprocess.run(["git", "worktree", "remove", "--force",
                            os.path.join(tmp_parent, "base")], cwd=repo)
            shutil.rmtree(tmp_parent, ignore_errors=True)
    sys.exit(rc)


if __name__ == "__main__":
    main()
