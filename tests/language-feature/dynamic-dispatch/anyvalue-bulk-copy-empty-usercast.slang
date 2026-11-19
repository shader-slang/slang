//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -stage compute -entry computeMain

// A user-authored `bit_cast` from an empty struct to a nonempty one is a size mismatch (0 payload
// bytes into 4). Empty-type legalization removes `Empty` before `lowerBitCast` runs, so the
// mismatch is reported during type legalization instead, at the `bit_cast` expression. The AnyValue
// bulk-copy zero-fill applies only when the result is the generated `AnyValueN` storage struct
// (marked with `IRAnyValueSizeDecoration`), so this cast to a plain `Word` is never silently
// rewritten to `Word{0}`.

struct Empty
{
}

struct Word
{
    uint u;
}

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Empty e;
    Word w = bit_cast<Word>(e);
//CHECK:                   ^ E41202
//CHECK:                   ^ 'Empty' with size '0' casted into 'Word' with size '4'
    outputBuffer[0] = w.u;
}
