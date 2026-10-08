// `legalizeResourceGlobalVars` can replace a plain file- or namespace-scope
// `static RWStructuredBuffer` with per-function storage and generated call arguments. Function-local
// storage cannot provide `groupshared` visibility or preserve the access guarantees requested by
// `globallycoherent`.
// Structs also remain unsupported because the replacement pass does not split an aggregate or
// coordinate separate replacements for its fields.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
RWStructuredBuffer<uint> outputBuffer;

RWStructuredBuffer<uint> inputBuffer;

static RWStructuredBuffer<uint> gBuffer = inputBuffer;

static groupshared RWStructuredBuffer<uint> gSharedBuffer;
//CHECK:                                    ^^^^^^^^^^^^^ mutable static resource cannot use shared or memory-qualified storage
//CHECK:                                    ^^^^^^^^^^^^^ a resource-valued `static` variable declared at file or namespace scope cannot use `groupshared` or a memory-access qualifier

globallycoherent static RWStructuredBuffer<uint> coherentBuffer = inputBuffer;
//CHECK:                                         ^^^^^^^^^^^^^^ mutable static resource cannot use shared or memory-qualified storage
//CHECK:                                         ^^^^^^^^^^^^^^ a resource-valued `static` variable declared at file or namespace scope cannot use `groupshared` or a memory-access qualifier

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
             ^^^^^^ opaque type is not supported for mutable static storage
             ^^^^^^ mutable file- or namespace-scope `static` variable 'gStuff' has the opaque type 'Stuff', which cannot be represented by mutable storage
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
