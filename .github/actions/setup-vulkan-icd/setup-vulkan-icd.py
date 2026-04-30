"""Write a Vulkan ICD JSON override pointing at libEGL_nvidia.so.0.

Reads the NVIDIA ICD JSON injected by the container toolkit (if present),
swaps the library_path to libEGL_nvidia.so.0, and writes the result to
the path given in $ICD_OUT. If no source ICD is found, a minimal JSON
with a conservative api_version is written.

This avoids a libEGL_nvidia.so crash observed on NVIDIA driver 580.x
when the driver is loaded via the default libGLX_nvidia.so.0 ICD. See
.github/workflows/ci-slang-test-container.yml job-level comment for
details.
"""

import json
import os
import sys


def main() -> int:
    source = os.environ.get("ICD_SOURCE", "")
    out = os.environ["ICD_OUT"]

    if source:
        with open(source) as f:
            icd = json.load(f)
        icd.setdefault("ICD", {})
    else:
        icd = {"file_format_version": "1.0.1", "ICD": {"api_version": "1.4.312"}}

    icd["ICD"]["library_path"] = "libEGL_nvidia.so.0"

    with open(out, "w") as f:
        json.dump(icd, f, indent=4)
        f.write("\n")

    api_version = icd["ICD"].get("api_version", "unknown")
    print(f"Vulkan ICD configured: libEGL_nvidia.so.0 (api_version={api_version})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
