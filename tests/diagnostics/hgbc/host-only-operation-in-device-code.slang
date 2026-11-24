//TEST:SIMPLE(filecheck=INLINED): -target spirv -entry inlined -stage compute
//TEST:SIMPLE(filecheck=CALLED): -target spirv -entry called -stage compute

// BACKSTOP test for `checkHostOnlyInstsInDeviceCode`. Host-only `hgbc_asm` reaching device code
// generation is reported as E56050 at the source line instead of crashing the emitter. The
// capability system should reject both cases first; when it does, this test should change to
// expect the capability error, and the backstop can be removed.

RWStructuredBuffer<uint> output;

// `alloc` is a `[require(hgbc)]` static method of a generic type, whose requirement does not
// currently reach the entry point. Its `hgbc_asm` block is inlined at the call.
[shader("compute")]
[numthreads(1, 1, 1)]
void inlined(uint3 id: SV_DispatchThreadID)
{
    let scratch = RWStructuredBuffer<uint>::alloc(4);
    output[0] = scratch[0];
}

// INLINED: error[E56050]: host-only operation in device code
// INLINED: host-only-operation-in-device-code.slang:17:
// INLINED-NOT: Unhandled local inst

// `readBack` is reached through a generic `IFunc` argument, whose requirements do not currently
// reach the entry point. Its inlined block keeps the library's location, so the error points at
// the line in this file that uses its result.
struct ReadingCallback : IFunc<void, uint>
{
    void operator()(uint i) { output[i] = output.readBack(0); }
}

void invoke<F : IFunc<void, uint>>(F f, uint i) { f(i); }

[shader("compute")]
[numthreads(1, 1, 1)]
void called(uint3 id: SV_DispatchThreadID)
{
    ReadingCallback callback;
    invoke(callback, id.x);
}

// CALLED: error[E56050]: host-only operation in device code
// CALLED: host-only-operation-in-device-code.slang:30:
// CALLED-NOT: hlsl.meta.slang
// CALLED-NOT: Unhandled local inst
