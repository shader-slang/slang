# TinyEXR

Vendored from [TinyEXR v1.0.13](https://github.com/syoyo/tinyexr/tree/4946b5d92e13bcc8102ac2c8efd129596a90bf75)
(commit `4946b5d92e13bcc8102ac2c8efd129596a90bf75`). The three headers are unmodified.
See LICENSE for BSD-3-Clause terms and embedded notices for contributing code.

HGVM uses this single-header C++ release for EXR scanline decoding, linked against the existing
miniz dependency. HGVM frames one image from its forward-only byte stream before passing the
encoded image to TinyEXR. No network fetch is performed during configuration or at runtime.
