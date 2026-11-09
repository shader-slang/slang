# Standalone host compute examples

These programs use the standard modules bundled with the h-slang compiler. They do not
require external libraries, assets, or an additional module search path. Run them from this directory:

```sh
slangi -api vulkan grid-dispatch.slang
slangi -api vulkan buffer-texture-utils.slang
slangi -api vulkan tensor-affine-fit.slang
```

The grid and buffer/texture examples also support `-api d3d12` on Windows.
Use Vulkan or CUDA for the affine fit: its captured parameter gradients require
floating-point atomic addition, which the current D3D12 shader target cannot compile.
`grid-dispatch` prints `513.000000`; `buffer-texture-utils` prints `438.000000` and `342.000000`.
The affine fit learns gain 2 and bias 1 through differentiable tensor operations.

Explicit imports select the APIs:

| Import                        | API                                                                          |
| ----------------------------- | ---------------------------------------------------------------------------- |
| `slang.runtime`               | Runtime helpers, I/O, resources, dispatch, raster, ray tracing, window/input |
| `slang.tensor`                | GPU tensor storage, fill, copy, map, reduction, inner products               |
| `slang.tensor.differentiable` | Tensor API plus gradient allocation and custom derivatives                   |
| `slang.compute.buffer_utils`  | Buffer fill/zero, map, and reduction                                         |
| `slang.compute.texture_utils` | Texture fill, map, and reduction                                             |

Focused imports such as `slang.runtime.compute.grid` remain available. Keep the compiler's
versioned `slang-standard-module-*` directory beside its library when relocating the tools.
`slangi` enables experimental features; `slangc` and compiler API clients must enable them
when using the experimental numeric interfaces or custom derivative syntax.

See the [resource guide](../../docs/heterogeneity/resources.md) and
[automatic differentiation guide](../../docs/heterogeneity/automatic-differentiation.md).
