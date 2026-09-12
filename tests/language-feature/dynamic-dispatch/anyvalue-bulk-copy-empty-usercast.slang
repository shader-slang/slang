//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain

// A user-authored `bit_cast` from an empty struct to a nonempty one is a size mismatch (0 payload
// bytes into 4). Empty-type legalization lowers the empty source to `none`, so the cast reaches
// type legalization with a non-simple operand and must fail loudly rather than compile. The
// AnyValue bulk-copy empty-source zero-fill only applies when the cast's result is the concrete
// `AnyValueN` storage struct (marked with `IRAnyValueSizeDecoration` by the marshalling pass), so
// this cast to a plain user `Word` is never silently rewritten to `Word{0}`.

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
    outputBuffer[0] = w.u;
}

// CHECK: E99997{{.*}}non-simple operand
