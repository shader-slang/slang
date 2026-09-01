# Slice 181: Establish an explicit NVVM-ready IR contract

## Motivation

Compile-time CUDA layout calls and source-level marker operations reached direct preflight as if
they were LLVM instructions. This made the emitter own cleanup decisions and caused valid compute
kernels such as `matrix-double` to stop on the read-none `unmodified` source check.

## Proposed solution

Insert one named direct-NVVM legalization stage after linked-IR normalization and before preflight.
Let it reduce only operations whose semantic purpose is proven complete, then verify its own
postcondition. Keep any marker with live semantics unsupported.

## Change summary

- Added `slang-ir-nvvm-legalize.*` and invoked `legalizeIRForNVVM` at the direct handoff.
- Moved CUDA size/alignment/offset folding and dead initializer cleanup out of the emitter.
- Removed canonical void `unmodified` and CUDA `RequireComputeDerivative` operations.
- Retained live `RequirePrelude` and exposed the texture sample's next real GenericAsm blocker.
- Promoted `matrix-double` to permanent O0/O3 direct regression coverage.

## Concepts and vocabulary

**NVVM-ready IR** is linked, CUDA-specialized Slang IR after representation-only CUDA operations
have been consumed but before provider discovery. **Postcondition** is the deterministic scan that
ensures an operation owned by this legalizer cannot leak into preflight.

## Process report

Consider this code from `matrix-double.slang`:

```slang
void test1(inout FloatMatrix ft, inout FloatMatrix f, int idx)
{
    unmodified(f);
    ft += FloatMatrix(IntMatrix(((f % makeFloatMatrix(0.11f)) * 100) + 0.5));
}
```

Core declares `unmodified(out T)` as a read-none, force-inlined `kIROp_Unmodified` operation. It
exists to silence the definite-assignment check, returns void, and has no executable effect. The
legalizer accepts only the canonical one-operand/no-use shape and removes it. Any value-producing
or differently shaped spelling remains a deterministic error. With that operation removed,
`matrix-double` compiles and runs correctly at O0 and O3 and now owns permanent directives.

`cuda-texture.slang` reaches `RequireComputeDerivative` because a sampled texture operation needs
derivatives. GLSL consumes that marker by adding an entry-point execution mode; CUDA's common
pipeline has already admitted the operation and the CUDA emitter intentionally emits nothing for
the marker. Direct legalization therefore removes it. The fixture then stops on the exact ordinary
texture-sample GenericAsm, proving this rewrite exposes rather than masks the next unsupported
semantic operation.

`cuda/require-prelude.slang` is the negative ownership case. Its marker defines
`MY_CUDA_INTRINSIC`, and the following GenericAsm refers to that macro. The prelude is therefore
live source semantics, not a representation-only marker. It remains the first deterministic
preflight failure before builder discovery; no fallback or textual macro expansion was added.

CUDA layout queries already had a compiler-owned fold using shared CUDA layout rules and exact
field keys. That complete implementation moved to the legalizer along with its narrow dead
synthesized-initializer cleanup. Emission no longer exposes a separate fold API. The postcondition
rejects any residual layout-query call, `unmodified`, or `RequireComputeDerivative`.

The self-review inventory contains the new stage, three exact rewrites, DCE, postcondition, and
test promotion. Every rewrite names its producer and consumer above. No fixture-name check,
syntax reconstruction, compatibility path, provider callback, or malformed-upstream repair was
added. Provider ABI revision 34 is unchanged.

Frozen corpus v1 remains exactly 452 workloads/427 healthy references and advances from
417/417/417 to 418/418/418 O0/O3/both, with one gain and no old-correct loss. All-row direct
classifications are 432 correct, three runtime mismatches, and 17 preflight failures per mode.
Discovery remains exactly 82 workloads/72 healthy references at 72/72/72. The selected prefix
passes 437/437 and the permanent NVVM category passes 92/92.
