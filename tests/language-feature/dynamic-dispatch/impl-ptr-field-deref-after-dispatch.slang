// Dereference a pointer field after the containing struct has been packed into
// an existential and then unpacked through dynamic dispatch. The critical code
// path is AnyValue pack (bitcast pointer to uint/uint2) followed by unpack
// (bitcast back to pointer type). The resulting pointer must still reference
// the original memory.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

interface IReader
{
    int read();
}

struct IndirectReader : IReader
{
    int* source;
    int offset;

    int read()
    {
        return *source + offset;
    }
}

struct DirectReader : IReader
{
    int value;

    int read()
    {
        return value;
    }
}

// Pack into existential by returning IReader
IReader makeReader(int id, int* pSource)
{
    if (id == 0)
    {
        IndirectReader r;
        r.source = pSource;
        r.offset = 10;
        return r;
    }
    else
    {
        DirectReader d;
        d.value = 999;
        return d;
    }
}

// Dispatch through the interface -- forces unpack of the existential.
// IndirectReader::read dereferences its pointer field after unpack.
int dispatchRead(IReader reader)
{
    return reader.read();
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int val = 32;

    // IndirectReader: *source + offset = 32 + 10 = 42
    var reader = makeReader(0, &val);
    // CHECK: 42
    outputBuffer[0] = dispatchRead(reader);

    // DirectReader: value = 999
    var reader2 = makeReader(1, nullptr);
    // CHECK: 999
    outputBuffer[1] = dispatchRead(reader2);

    // Modify the source value and re-dispatch through the original reader.
    // The pointer stored in the already-packed existential must still
    // reference the original memory and reflect the updated value.
    val = 90;
    // CHECK: 100
    outputBuffer[2] = dispatchRead(reader);
}
