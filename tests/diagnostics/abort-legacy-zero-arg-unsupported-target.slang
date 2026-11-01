// The zero-argument abort() requires cpp_hlsl, so calling it from an entry point compiled for any
// other target reports E36107. (The variadic abort(format, ...) is the GLSL/SPIR-V form; see
// abort-unsupported-target.slang.)

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target metal -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target glsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target cuda -entry computeMain -stage compute

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x > 100)
    {
        abort();
    }
    outputBuffer[0] = 1;
}

// Only the capability error: no attempt to parse an `abort` intrinsic snippet (E29000).
//CHECK-NOT: E29000
//CHECK: E36107
//CHECK-NOT: E29000
