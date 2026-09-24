# Slice 210: preserve small double source literals

## Motivation

Consider this source fragment, whose input buffer contains independently supplied IEEE binary64 words:

```slang
uint2 bits = inputBits[lane];
double actual = asdouble(bits.x, bits.y);
bool ok = actual == 0x1.0000000400000p-16l;
outputBuffer[lane] = ok ? 8000 + lane : 0;
```

The literal is exactly `(1 + 2^-30) / 65536`. Source emission spelled it as
`0.00001525878907671`, which represents a different double. The old CUDA/NVRTC execution fails
this check while direct NVVM O0/O3 pass. Slice 208 exposed the same defect in an aggregate
expectation; correcting the shared writer protects the trusted NVRTC reference used by the loop.

## Proposed solution

Retain the existing classic-locale stream, precision and decimal mantissa trimming. Select
scientific notation for binary exponents below zero. The remaining nonzero fixed-format range
starts at magnitude 0.5, where 17 fractional digits supply at least 17 significant digits.
Scientific precision supplies enough significant digits for every smaller finite double. Zero
keeps its old fixed spelling and the existing large-value scientific threshold stays unchanged.

## Change summary

- `source/slang/slang-emit-source-writer.cpp`: correct fixed/scientific selection at the shared
  value-to-decimal boundary, with a concrete explanation of the precision requirement.
- New compiler API unit: exact-bit roundtrip of independently chosen binary64 patterns through
  HLSL, GLSL, CUDA and C++ emission, including all finite exponents and signed boundary values.
- New CUDA runtime fixture: integer-word input makes expected data independent of source literals
  and prevents a folded constant comparison from concealing the defect; all three modes execute.
- Discovery adds one source, preserving every old contract and frozen identity. Plan, report,
  manifest, census, design and STATUS record the full shared-boundary checkpoint.

## Concepts and vocabulary

`max_digits10` is the decimal significant-digit count sufficient to recover a binary floating
value. Stream fixed precision counts digits after the decimal point, including leading fractional
zeros; scientific precision counts fractional digits of a normalized mantissa. The exponent from
`frexp` satisfies `value = mantissa * 2^exponent` with absolute mantissa in `[0.5, 1)` for nonzero
finite values. A binary64 bit oracle avoids introducing another potentially rounded decimal value.

## Process report

`visitFloatingPointLiteralExpr` lowers the checked literal through `IRBuilder::getFloatValue`.
For double, that builder stores the input unchanged in the canonical `IRConstant` float payload.
`CLikeSourceEmitter::emitSimpleValueImpl` sends the `kIROp_FloatLit` payload to
`SourceWriter::emit(double)`. The producer and IR are correct: direct NVVM consumes the same
semantic value and passes the independent runtime oracle. The defect belongs to the shared
writer's decimal formatting contract, not to parsing, lowering, CUDA semantics, or GPU arithmetic.

The change inventory contains one existing selection rule and its comment. There are no new
production helpers, fallbacks, equivalence rules, syntax reconstruction, graph walks or semantic
representations. Small finite double constants are canonical valid input. The writer already
owns formatting, so fixing its significant-digit guarantee is the responsible layer. A proposed
unqualified defaultfloat switch was rejected because existing trimming assumes a decimal point;
retaining existing formatting machinery also avoids unnecessary spelling changes to larger values.

The API test feeds hexadecimal constants derived from independent integer patterns, then parses
emitted decimal stores in the classic locale and compares every bit. A custom comma locale is
active during compilation, without relying on locally installed locales. It covers both signed
zeros, subnormal and normal boundaries, neighbors of the old/new formatting transitions, values
beyond float32 precision, maximum finite magnitude and every finite exponent. Syntax checks also
require a decimal point and reject locale commas. The locale guard restores caller state.

The GPU test reads host-provided integer words and compares their runtime double interpretation
against source constants of both signs. This preserves data dependence through optimization.
Its first motivating lane fails with the old writer and succeeds after the change; direct NVVM
already passes. Old corpus fixtures and output expectations remain untouched.

Full validation is complete; exact final identities and evidence are recorded in
`runtime-validation.slice-210.json`. Parent completed acceptance and owns the local commit. Correctness overrides complex cadence because the application
runtime bindings, inputs and oracle for the six compiling material cells remain unavailable.

### Final validation and preservation

| Gate                                     | Result                                                                                          |
| ---------------------------------------- | ----------------------------------------------------------------------------------------------- |
| Focused                                  | 4/4: one API unit (2127 patterns x 4 targets), three GPU cells                                  |
| Runtime smoke                            | 4/4                                                                                             |
| NVVM/routing/reporting plus new API unit | 476/476; one existing Windows-only skip                                                         |
| Representative source-target regressions | 16/16; three unavailable platform lanes skipped                                                 |
| Toolkit                                  | 18/18 compile/assembly cells                                                                    |
| Full frozen                              | 1356 fresh cells: 1333 correct, 23 unchanged failures                                           |
| Full discovery                           | 291 fresh cells: 261 correct, 30 unchanged failures                                             |
| Complex materials                        | 6/6 compile/assembly cells; no runtime claim                                                    |
| Structural probes                        | Four before/after source targets; all three PTX modes retain runtime loads and FP64 comparisons |

The full checkpoint has 1647 fresh cells: 1594 correct and 53 retained failures. All 1591 prior
correct cells are preserved. The three additions pass; every old identity/mode, classification,
return code, full execution counts, diagnostic and canonical shape matches accepted 209 exactly.
There are no missing/duplicate cells, inherited runtime cells, old fixes or new regressions. Both
full runners return expected diagnostic exit 2. The ledger retains all 53 failure histories and all
four resolved 208 histories. Historical healthy denominators remain 427 frozen and 72 discovery.
Discovery grows 96 to 97 distinct sources; frozen 452 identities and every old oracle remain unchanged.

The before host run reported 92 bit mismatches across the four source targets with no parse/syntax
failure. Before GPU results were NVRTC mismatch and NVVM O0/O3 correct. Afterward all pass.
The motivating spelling is now `1.52587890767108547e-05`. Custom locale, signed zero, fixed and
scientific trimming, every finite exponent and tiny normal/subnormal values are covered. Shared
regression skips are existing DX11 Inf, downstream Metal suffix and WGPU Inf lanes; no claim is
made that those unavailable downstream platforms executed.

Final compiler SHA256: `f8dc709857e70fbf7e4c0bbe2d94f9c1528f789ddf609a4c0db6f5bb90b776c7`.
Provider SHA256 (ABI 36): `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Tested source base: `1f161256b4bc8f57aa96bc0dd250f8e2a537ef1c` plus manifest source hashes.
A comment-only precision-bound clarification triggered a rebuild and complete gate restart before
full corpus execution; the compiler hash remained identical. All final tests use the final source
and recorded artifacts. Final hash checks pass. No GPU loss, driver change, reboot, commit or push.
Raw artifacts remain under `build/nvvm-loop/slice-210-{before,after}`.

Parent accepted 210 as the latest full checkpoint; implementation cadence remains 0.
Rolling 208 FP64 wave arithmetic, 209 active masks and 210 source literals prioritize demonstrated
correctness. Reconsider complex motivation before the next slice without inventing application
runtime contracts. No next independent feature was investigated here.

Parent accepted the final full checkpoint after independent code, oracle, hash and preservation
review. The latest full checkpoint is 210 and the implementation cadence is 0.
