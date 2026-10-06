#!/usr/bin/env python3
"""Check real Vulkan coverage collection, batching, and export in both RHI demos.

Build shader-coverage-image-pipeline and shader-coverage-bvh-traversal first.
Run from any directory; pass --bin-dir for a non-default build location.
Use --counter-width=64 only on a device with 64-bit buffer atomics.
"""

import argparse
import json
from pathlib import Path
import subprocess
import sys
import tempfile


def run(command):
    result = subprocess.run(command, text=True, capture_output=True, timeout=180)
    if result.returncode:
        raise RuntimeError(
            f"Command failed ({result.returncode}): {' '.join(map(str, command))}\n"
            f"{result.stdout}\n{result.stderr}"
        )
    return result.stdout


def read_counters(directory, mode, width):
    manifest = json.loads((directory / f"{mode}.coverage-manifest.json").read_text())
    raw = (directory / f"{mode}.counters.bin").read_bytes()
    stride = manifest["buffer"]["element_stride"]
    assert stride == width // 8, (stride, width)
    assert len(raw) == manifest["counter_count"] * stride
    counters = [int.from_bytes(raw[i:i + stride], "little") for i in range(0, len(raw), stride)]
    assert any(counters), "Coverage buffer was never written"
    assert any(value == 0 for value in counters), "Expected an unexercised shader path"
    return manifest, counters


def covered_entries(manifest, counters):
    # Counter slots may be coalesced; compare source attribution, not slot numbers.
    covered = set()
    for entry in manifest["entries"]:
        index = entry["counter"]
        if index is not None:
            assert 0 <= index < len(counters)
            if counters[index]:
                location = {key: value for key, value in entry.items() if key not in ("counter", "mode")}
                covered.add(json.dumps(location, sort_keys=True))
    return covered


def main():
    root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bin-dir", type=Path, default=root / "build" / "Release" / "bin")
    parser.add_argument("--counter-width", type=int, choices=(32, 64), default=32)
    args = parser.parse_args()
    suffix = ".exe" if sys.platform == "win32" else ""
    converter = root / "tools" / "shader-coverage" / "slang-coverage-to-lcov.py"
    demos = [
        ("image-pipeline", ["--width=24", "--height=17"], "--tile-rows", 8),
        ("bvh-traversal", ["--ray-grid-size=17"], "--batch-size", 64),
    ]
    runs = 0
    with tempfile.TemporaryDirectory(prefix="slang-rhi-coverage-") as temp:
        for name, dimensions, batch_flag, batch_size in demos:
            binary = (args.bin_dir / f"shader-coverage-{name}{suffix}").resolve()
            shader_dir = root / "examples" / f"shader-coverage-{name}"
            base = [str(binary), *dimensions, f"--demo-dir={shader_dir}",
                    f"--counter-width={args.counter_width}"]
            for mode in ("smoke", "full"):
                hits = {}
                for recording in ("count", "boolean"):
                    results = []
                    for batch in (0, batch_size):
                        output = Path(temp) / f"{name}-{mode}-{recording}-{batch}"
                        run([*base, f"--mode={mode}", f"--coverage-mode={recording}",
                             f"{batch_flag}={batch}", f"--output-dir={output}"])
                        runs += 1
                        manifest, counters = read_counters(output, mode, args.counter_width)
                        if recording == "boolean":
                            assert set(counters) <= {0, 1}, "Boolean counters must be hit flags"
                        run([sys.executable, str(converter), "--manifest",
                             str(output / f"{mode}.coverage-manifest.json"), "--counters",
                             str(output / f"{mode}.counters.bin"), "--output", str(output / "report.lcov")])
                        report = (output / "report.lcov").read_text()
                        assert "DA:" in report and "BRDA:" in report and "FNDA:" in report
                        results.append((manifest, counters))
                    assert results[0] == results[1], f"Batching changed coverage: {name}/{mode}/{recording}"
                    hits[recording] = covered_entries(*results[0])
                assert hits["count"] == hits["boolean"], f"Recording mode changed hit locations: {name}/{mode}"
                output = Path(temp) / f"{name}-{mode}-disabled"
                run([*base, f"--mode={mode}", "--no-coverage", f"{batch_flag}={batch_size}",
                     f"--output-dir={output}"])
                runs += 1
                assert not output.exists(), "Coverage-disabled run wrote coverage artifacts"
                print(f"PASS {name}/{mode}: batching, count/boolean, export, coverage disabled", flush=True)
    print(f"PASS {runs} Vulkan demo runs ({args.counter_width}-bit counters)")


if __name__ == "__main__":
    main()
