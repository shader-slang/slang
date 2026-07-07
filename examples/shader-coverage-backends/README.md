# shader-coverage-backends

One shader, three coverage dispatch paths. This example runs the user
guide coverage tutorial's kernel
([`docs/user-guide/a1-06-shader-coverage.md`](../../docs/user-guide/a1-06-shader-coverage.md))
for real on a selectable backend, demonstrating that the coverage
workflow — compile, discover the hidden counter buffer through
`ISyntheticResourceMetadata`, bind, dispatch, attribute counters
through `ICoverageTracingMetadata` — is identical everywhere, and that
only the binding step's shape changes per backend:

| Backend  | Binding model exercised                                                                             |
| -------- | --------------------------------------------------------------------------------------------------- |
| `cpu`    | host-callable kernel; `(pointer, count)` pair written into the parameter payload at `uniformOffset` |
| `vulkan` | storage buffer at the descriptor `(space, binding)`; auto-allocation adds one descriptor set        |
| `metal`  | `[[buffer(N)]]` index from `binding`; MSL compiled at runtime, counters always 32-bit               |

Each path is a compact implementation of the corresponding recipe in
[`docs/design/shader-coverage-host-interface.md`](../../docs/design/shader-coverage-host-interface.md).
For coverage-driven _analysis_ workflows on realistic kernels, see the
sibling examples `shader-coverage-image-pipeline` and
`shader-coverage-bvh-traversal`.

## Running

```bash
shader-coverage-backends --backend=cpu
shader-coverage-backends --backend=vulkan
shader-coverage-backends --backend=metal      # Apple platforms
```

Every run dispatches four threads over the inputs `{1, -2, 3, 4}`,
verifies the outputs, and prints the per-line hit table. The inputs are
chosen to produce partial coverage on purpose:

```
[cpu] line coverage of hello-coverage.slang:
  line 14: 4
  line 15: 4
  line 16: 0   <-- never executed
  line 23: 4
  ...
  line 26: 1
  line 27: 4
```

The three backends produce identical counter values for the same
inputs — line 16 is `applyGain`'s `return value;` fallthrough (the
gain is always 2.0), and line 26 is the negative-input clamp, which
runs exactly once.

Each run also writes `<backend>.coverage-manifest.json` and
`<backend>.counters.bin`, ready for the LCOV pipeline:

```bash
python3 tools/shader-coverage/slang-coverage-to-lcov.py \
    --manifest cpu.coverage-manifest.json --counters cpu.counters.bin \
    --output cpu.lcov
genhtml cpu.lcov --output-directory coverage-html
```

## Options

- `--counter-width=32|64` — counter element width (default 32, which
  runs everywhere). `64` requires 64-bit shader atomics on the device
  for the Vulkan path (`shaderBufferInt64Atomics` — absent on MoltenVK
  on Apple Silicon); Metal always caps to 32-bit (warning W45115 when
  64 is requested explicitly). The example reads the _effective_ width
  back from `CoverageBufferInfo::elementByteWidth` rather than trusting
  the request — do the same in your integration.
- `--demo-dir=<path>` — directory containing `hello-coverage.slang`,
  when running from outside the source tree.

## Build requirements

- CPU path: a C++ toolchain Slang can use for host-callable
  compilation (present in normal development setups).
- Vulkan path: a Vulkan loader at configure time (headers come from
  Slang's bundled Vulkan-Headers). Without it, the example still
  builds with the Vulkan path disabled.
- Metal path: Apple platforms; no external Metal toolchain needed
  (the MSL is compiled by the Metal framework at runtime).
