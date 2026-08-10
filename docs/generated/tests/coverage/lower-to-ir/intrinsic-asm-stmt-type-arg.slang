//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies an `__intrinsic_asm "...", T;` statement lowers a type argument as an IR type and substitutes the target's spelling of that type into the `$[0]` slot.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the type-argument arm of `visitIntrinsicAsmStmt`: an argument whose
// checked type is a `TypeType` is lowered with `lowerType` and appended to the
// GenericAsm inst as an IR type rather than as a value. The `$[0]` slot then
// expands to the target's own spelling of the type, so compiling
// `mySizeOf<float4>()` to C++ must produce the CUDA/CPU prelude's
// `Vector<float, 4>` name -- something only reachable if the generic argument
// arrived as a lowered type.
//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

int mySizeOf<T>()
{
    __intrinsic_asm "(int)sizeof($[0])", T;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = mySizeOf<float4>();
}

// The `$[0]` slot expands to the emitted C++ spelling of `float4`.
//CHECK: ((int)sizeof(Vector<float, 4>))
