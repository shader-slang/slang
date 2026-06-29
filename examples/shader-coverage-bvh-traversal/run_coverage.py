#!/usr/bin/env python3
"""
run_coverage.py — end-to-end wrapper for the shader-coverage-bvh-traversal demo.

Runs the demo binary, converts raw counters to a rich LCOV (branch +
function records), renders an HTML report with Slang's built-in renderer,
and opens it in the default browser.

Usage:
    python3 run_coverage.py [DEMO FLAGS]
    python3 run_coverage.py --mode=full --coverage-mode=boolean
    python3 run_coverage.py --mode=smoke --output-dir=./out

All flags are forwarded verbatim to the demo binary.  Extra flags:
    --slang-root PATH   Root of the Slang repo (auto-discovered by default).
    --no-open           Render the report but do not open it.
    --help              Show this message and exit.
"""

import argparse
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path


# ---------------------------------------------------------------------------
# Slang-root discovery
# ---------------------------------------------------------------------------

def _find_slang_root(hint: Path | None) -> Path:
    """Walk up from this script's location to find the Slang repo root."""
    if hint is not None:
        root = Path(hint).resolve()
        if not (root / "tools" / "coverage-html" / "slang-coverage-html.py").exists():
            sys.exit(f"error: --slang-root {root} does not look like a Slang repo "
                     "(tools/coverage-html/slang-coverage-html.py not found)")
        return root
    here = Path(__file__).resolve()
    for parent in [here.parent, *here.parents]:
        if (parent / "tools" / "coverage-html" / "slang-coverage-html.py").exists():
            return parent
    sys.exit(
        "error: cannot locate the Slang repo root.\n"
        "Run from inside the Slang source tree or pass --slang-root=<path>."
    )


# ---------------------------------------------------------------------------
# Demo binary discovery + auto-build
# ---------------------------------------------------------------------------

_TARGET = "shader-coverage-bvh-traversal"


def _candidate_paths(slang_root: Path) -> list:
    """All plausible output locations for the demo binary."""
    return [
        # CMake multi-config (Ninja Multi-Config / Visual Studio)
        *(slang_root / "build" / "examples" / _TARGET / config / _TARGET
          for config in ("Release", "Debug", "RelWithDebInfo")),
        # Single-config generators
        *(slang_root / "build" / config / "examples" / _TARGET / _TARGET
          for config in ("Release", "Debug", "RelWithDebInfo")),
        slang_root / "build" / "examples" / _TARGET / _TARGET,
        slang_root / "build" / "examples" / _TARGET / (_TARGET + ".exe"),
        # Windows .exe variants (multi-config)
        *(slang_root / "build" / "examples" / _TARGET / config / (_TARGET + ".exe")
          for config in ("Release", "Debug", "RelWithDebInfo")),
        # Windows .exe variants (single-config)
        *(slang_root / "build" / config / "examples" / _TARGET / (_TARGET + ".exe")
          for config in ("Release", "Debug", "RelWithDebInfo")),
    ]


def _is_stale(binary: Path, slang_root: Path) -> bool:
    """True if any source file in the demo directory is newer than the binary."""
    binary_mtime = binary.stat().st_mtime
    demo_dir = slang_root / "examples" / _TARGET
    for pattern in ("*.cpp", "*.h", "*.slang"):
        for src in demo_dir.glob(pattern):
            if src.stat().st_mtime > binary_mtime:
                return True
    return False


def _ensure_demo_binary(slang_root: Path) -> Path:
    """Return the demo binary path, building it first if necessary."""
    # Fast path: already built and up to date.
    for c in _candidate_paths(slang_root):
        if c.exists():
            if _is_stale(c, slang_root):
                print(f"[build] binary is stale (source newer than binary) — rebuilding")
                break
            return c

    # Slow path: build only the demo target. Assumes Slang itself is already
    # built (cmake --build --preset release was run by the user).
    print(f"[build] building target '{_TARGET}' …")
    result = subprocess.run(
        [shutil.which("cmake.exe") or "cmake",
         "--build", "--preset", "release", "--target", _TARGET],
        cwd=slang_root,
    )
    if result.returncode != 0:
        sys.exit(
            f"error: cmake build failed for target '{_TARGET}'.\n"
            "Build Slang first:  cmake --build --preset release"
        )

    for c in _candidate_paths(slang_root):
        if c.exists():
            return c

    sys.exit(
        f"error: build succeeded but '{_TARGET}' binary not found in expected paths.\n"
        f"Searched under: {slang_root / 'build'}"
    )


# ---------------------------------------------------------------------------
# Argument parsing
# ---------------------------------------------------------------------------

def _parse_args(argv):
    p = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        add_help=False,
    )
    p.add_argument("--slang-root", default=None,
                   help="Slang repo root (auto-discovered if omitted)")
    p.add_argument("--no-open", action="store_true",
                   help="Render the report but do not open it in a browser")
    p.add_argument("--help", "-h", action="store_true",
                   help="Show this help and exit")

    # Intercept to know artifact prefix; still forwarded to binary.
    p.add_argument("--output-dir", default=None)
    p.add_argument("--mode", default="smoke")

    known, demo_args = p.parse_known_args(argv)
    return known, demo_args


# ---------------------------------------------------------------------------
# Open browser cross-platform
# ---------------------------------------------------------------------------

def _open_browser(path: Path):
    url = path.as_uri()
    system = platform.system()
    if system == "Darwin":
        subprocess.run(["open", url], check=False)
    elif system == "Windows":
        os.startfile(str(path))  # type: ignore[attr-defined]
    else:
        subprocess.run(["xdg-open", url], check=False)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main(argv=None):
    if argv is None:
        argv = sys.argv[1:]

    known, demo_args = _parse_args(argv)

    if known.help:
        print(__doc__)
        return 0

    script_dir = Path(__file__).resolve().parent
    slang_root = _find_slang_root(Path(known.slang_root) if known.slang_root else None)
    binary = _ensure_demo_binary(slang_root)

    mode = known.mode
    output_dir = Path(known.output_dir).resolve() if known.output_dir else script_dir

    # ------------------------------------------------------------------
    # Step 1 — run the demo binary
    # ------------------------------------------------------------------
    binary_cmd = [str(binary), f"--mode={mode}",
                  f"--output-dir={output_dir}", *demo_args]
    print(f"[1/3] running demo: {' '.join(str(a) for a in binary_cmd)}")
    # On Windows the runtime DLLs (slang.dll etc.) live in
    # <build-root>/<config>/bin/, not next to the demo exe. Prepend that
    # directory to PATH so the loader finds them (otherwise STATUS_DLL_NOT_FOUND).
    # Multi-config layout: exe at <build>/examples/<target>/<config>/<exe>
    # Single-config layout: exe at <build>/<config>/examples/<target>/<exe>
    # In both cases binary.parents[3] is the build root and the config name
    # sits one level above the exe (multi) or two levels above (single).
    _WIN_CONFIGS = {"Release", "Debug", "RelWithDebInfo"}
    demo_env = os.environ.copy()
    if platform.system() == "Windows" and len(binary.parents) >= 4:
        config = (binary.parent.name if binary.parent.name in _WIN_CONFIGS
                  else binary.parents[2].name)
        dll_dir = binary.parents[3] / config / "bin"
        demo_env["PATH"] = str(dll_dir) + os.pathsep + demo_env.get("PATH", "")
    result = subprocess.run(binary_cmd, env=demo_env)
    if result.returncode != 0:
        sys.exit(f"error: demo exited with code {result.returncode}")

    # ------------------------------------------------------------------
    # Step 2 — convert raw counters to rich LCOV
    # (the demo writes a line-only .lcov; the converter adds branch +
    # function records from the manifest)
    # ------------------------------------------------------------------
    manifest = output_dir / f"{mode}.coverage-manifest.json"
    counters = output_dir / f"{mode}.counters.bin"
    rich_lcov = output_dir / f"{mode}.full.lcov"

    if not manifest.exists():
        sys.exit(f"error: expected manifest at {manifest} — was coverage disabled?")

    converter = slang_root / "tools" / "shader-coverage" / "slang-coverage-to-lcov.py"
    conv_cmd = [
        sys.executable, str(converter),
        "--manifest", str(manifest),
        "--counters", str(counters),
        "--output", str(rich_lcov),
    ]
    print(f"[2/3] converting to rich LCOV: {rich_lcov.name}")
    result = subprocess.run(conv_cmd)
    if result.returncode != 0:
        sys.exit(f"error: converter exited with code {result.returncode}")

    # ------------------------------------------------------------------
    # Step 3 — render HTML report
    # ------------------------------------------------------------------
    html_dir = output_dir / f"{mode}-html"
    renderer = slang_root / "tools" / "coverage-html" / "slang-coverage-html.py"
    render_cmd = [
        sys.executable, str(renderer),
        str(rich_lcov),
        "--output-dir", str(html_dir),
        "--title", f"bvh-traversal {mode}",
        "--source-root", str(slang_root),
    ]
    print(f"[3/3] rendering HTML report → {html_dir}")
    result = subprocess.run(render_cmd)
    if result.returncode != 0:
        sys.exit(f"error: renderer exited with code {result.returncode}")

    index = html_dir / "index.html"
    print(f"report: {index}")

    if not known.no_open:
        _open_browser(index)

    return 0


if __name__ == "__main__":
    sys.exit(main())
