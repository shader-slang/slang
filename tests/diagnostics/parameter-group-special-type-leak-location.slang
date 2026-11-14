// Regression test for https://github.com/shader-slang/slang/issues/11395
//
// Warnings E31106 / E31107 (special types leaking out of a parameter group)
// must carry a source location pointing at the offending member / parameter
// group, rather than being emitted with no location.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry computeMain

struct Mixed
{
    float4 color;
    Texture2D tex;
//CHECK:      ^^^ warning E31107
    SamplerState samp;
//CHECK:         ^^^^ warning E31107
}

ConstantBuffer<Mixed> cb;
//CHECK:              ^^ warning E31106

RWStructuredBuffer<float4> buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    buf[0] = cb.color;
}
