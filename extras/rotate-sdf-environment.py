"""Bake the SDF tutorial's lighting orientation into a new, losslessly shifted EXR.

Requires OpenEXR and numpy. Run from any directory; the source EXR is preserved.
"""

from pathlib import Path

import numpy as np
import OpenEXR


images = Path(__file__).resolve().parents[1] / "docs/heterogeneity/images"
source_path = images / "relax_inn_seaview_suite_1k.exr"
output_path = images / "relax_inn_seaview_suite_front_1k.exr"

with OpenEXR.File(str(source_path), separate_channels=True) as source:
    height, width = source.channels()["R"].pixels.shape
    if width != 2 * height or width % 4 != 0:
        raise ValueError("Expected a 2:1 panorama with a width divisible by four")

    # The window is near u=5/8. Move it to u=3/8: atan2(z, x) = -45 degrees,
    # halfway between +X and -Z (the direction from the sphere to the camera).
    # A horizontal wrap is a yaw rotation; whole pixels preserve every HDR value.
    shift = -width // 4
    for channel in source.channels().values():
        channel.pixels = np.roll(channel.pixels, shift, axis=1)
    source.header()["comments"] = (
        "SDF tutorial: horizontal wrap by -256 pixels at 1024x512 (-90 degrees "
        "of longitude). Window approximately 45 degrees from camera-facing -Z "
        "toward +X. Original HDR values and PIZ compression preserved."
    )
    source.write(str(output_path))

    # Verify the written EXR retains every shifted channel, including float precision.
    with OpenEXR.File(str(output_path), separate_channels=True) as rotated:
        for name, channel in source.channels().items():
            actual = rotated.channels()[name].pixels
            if actual.dtype != channel.pixels.dtype or not np.array_equal(actual, channel.pixels):
                raise RuntimeError(f"EXR round trip changed channel {name}")

print(f"Wrote {output_path.name}: {width}x{height}, shifted {shift} pixels; HDR values verified.")
