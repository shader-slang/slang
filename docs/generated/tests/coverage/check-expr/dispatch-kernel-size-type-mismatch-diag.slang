//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=`__dispatch_kernel` reports a type mismatch against `vector<uint,3>` for each of its two size arguments that is not a 3-component integer vector.
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes visitDispatchKernelExpr's argument validation. Both size
// arguments are checked with the same predicate — an integer-based vector whose
// element count folds to the constant 3 — and each failure is reported
// independently, so passing a scalar for one and a 2-component vector for the
// other yields two errors rather than one. The expected type in the message is
// always spelled `vector<uint,3>` regardless of which integer vector type was
// actually written, because the checker builds that type fresh for the
// diagnostic. Checking then continues into the ordinary higher-order-invoke
// path, which is why the surrounding call still resolves.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target torch

[CudaKernel]
void myKernel(TensorView<float> a) { }

[TorchEntryPoint]
void test(TensorView<float> a)
{
    __dispatch_kernel(myKernel, 1.0f, uint2(32, 1))(a);
//CHECK:                        ^^^^ E30019
//CHECK:                        ^^^^ expected an expression of type 'vector<uint,3>', got 'float'
//CHECK:                                   ^ expected an expression of type 'vector<uint,3>', got 'vector<uint,2>'
}
