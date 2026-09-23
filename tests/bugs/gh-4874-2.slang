// `legalizeResourceGlobalVars` can replace a plain file- or namespace-scope
// `static RWStructuredBuffer` with one entry-point local. A function-local variable cannot provide
// `groupshared` visibility or preserve the access guarantees requested by `globallycoherent`.
// Structs also remain unsupported because one source variable may contain several resource and
// non-resource fields.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
RWStructuredBuffer<uint> outputBuffer;

RWStructuredBuffer<uint> inputBuffer;

static RWStructuredBuffer<uint> gBuffer = inputBuffer;

static groupshared RWStructuredBuffer<uint> gSharedBuffer;
//CHECK:                                    ^^^^^^^^^^^^^ global variable cannot have opaque type
//CHECK:                                    ^^^^^^^^^^^^^ global variable cannot have opaque type.
//CHECK:                                    ^^^^^^^^^^^^^ do you intend to define a `uniform` parameter instead?

globallycoherent static RWStructuredBuffer<uint> coherentBuffer = inputBuffer;
//CHECK:                                         ^^^^^^^^^^^^^^ global variable cannot have opaque type
//CHECK:                                         ^^^^^^^^^^^^^^ global variable cannot have opaque type.

struct Stuff
{
    __init(RWStructuredBuffer<uint> a1, RWStructuredBuffer<uint> b2)
    {
        a = a1;
        b = b2;
    }

    RWStructuredBuffer<uint> a;
    RWStructuredBuffer<uint> b;
}

static Stuff gStuff = Stuff( inputBuffer, inputBuffer );
/*CHECK:
             ^^^^^^ global variable cannot have opaque type
             ^^^^^^ global variable cannot have opaque type.
*/

uint test(uint x)
{
    return gBuffer[x]
        + gStuff.a[x + 1] * 16
        + gStuff.b[x + 2] * 256;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    let tid = dispatchThreadID.x;
    let inVal = tid;
    let outVal = test(inVal);
    outputBuffer[tid] = outVal;
}
