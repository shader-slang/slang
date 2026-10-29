//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -stage compute -entry computeMain -g -minimum-slang-optimization

// Whether a 64-bit min or max has an unused original value is decided on the IR that reaches the
// Metal emitter. With `-g` and `-minimum-slang-optimization`, Slang keeps the never-read
// `original` local, so the three-argument form is diagnosed while the two-argument form still
// emits the void `atomic_min_explicit` (see atomic-64bit.slang).

RWStructuredBuffer<uint64_t> buffer;

[numthreads(64, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    uint64_t key = uint64_t(tid);
    InterlockedMin(buffer[0], key);
    uint64_t original;
    InterlockedMin(buffer[1], key, original);
//CHECK:          ^ E55204
//CHECK:          ^ whose original value is used
}
