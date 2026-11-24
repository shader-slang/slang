//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute -validate-ir
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -cpu -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -cuda -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -llvm -compute -output-using-type
//TEST(compute,vulkan):COMPARE_COMPUTE(filecheck-buffer=RESULT): -vk -compute -output-using-type
//TEST(compute,metal):COMPARE_COMPUTE(filecheck-buffer=RESULT): -metal -compute -output-using-type

// Resource-selection legalization must reconstruct unsized conversions around selected
// buffers. Exercise both branches before and after conversion, and through resource-returning
// helpers. Writes must reach the selected binding in all four families.
// WGSL cannot represent these conditional storage-buffer values even for sized buffers;
// resource-selection legalization currently runs only on Khronos targets.
//TEST_INPUT:ubuffer(data=[10], stride=4):name=valuesA
StructuredBuffer<uint> valuesA;
//TEST_INPUT:ubuffer(data=[20], stride=4):name=valuesB
StructuredBuffer<uint> valuesB;
//TEST_INPUT:ubuffer(data=[30], stride=4):name=bytesA
ByteAddressBuffer bytesA;
//TEST_INPUT:ubuffer(data=[40], stride=4):name=bytesB
ByteAddressBuffer bytesB;
//TEST_INPUT:ubuffer(data=[0 0], stride=4):name=writtenValuesA
RWStructuredBuffer<uint> writtenValuesA;
//TEST_INPUT:ubuffer(data=[0 0], stride=4):name=writtenValuesB
RWStructuredBuffer<uint> writtenValuesB;
//TEST_INPUT:ubuffer(data=[0 0], stride=4):name=writtenBytesA
RWByteAddressBuffer writtenBytesA;
//TEST_INPUT:ubuffer(data=[0 0], stride=4):name=writtenBytesB
RWByteAddressBuffer writtenBytesB;
//TEST_INPUT:ubuffer(data=[0], count=6, stride=4):out,name=output
RWStructuredBuffer<uint> output;

[noinline]
__UnsizedStructuredBuffer<uint> chooseValues(bool first)
{
    return first ? __UnsizedStructuredBuffer<uint>(valuesA)
                 : __UnsizedStructuredBuffer<uint>(valuesB);
}

[noinline]
__UnsizedRWStructuredBuffer<uint> chooseWrittenValues(bool first)
{
    return first ? __UnsizedRWStructuredBuffer<uint>(writtenValuesA)
                 : __UnsizedRWStructuredBuffer<uint>(writtenValuesB);
}

[noinline]
__UnsizedByteAddressBuffer chooseBytes(bool first)
{
    return first ? __UnsizedByteAddressBuffer(bytesA) : __UnsizedByteAddressBuffer(bytesB);
}

[noinline]
__UnsizedRWByteAddressBuffer chooseWrittenBytes(bool first)
{
    return first ? __UnsizedRWByteAddressBuffer(writtenBytesA)
                 : __UnsizedRWByteAddressBuffer(writtenBytesB);
}

void recordResult(uint index, uint phase)
{
    output[index * 3 + phase] = writtenValuesA[index] + 10 * writtenValuesB[index]
        + writtenBytesA.Load(index * 4) + 10 * writtenBytesB.Load(index * 4);
}

[numthreads(2, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint index = tid.x;
    bool first = index == 0;

    // Select the sized resource first, then convert the result.
    let read0 = __UnsizedStructuredBuffer<uint>(first ? valuesA : valuesB);
    let write0 = __UnsizedRWStructuredBuffer<uint>(first ? writtenValuesA : writtenValuesB);
    let readBytes0 = __UnsizedByteAddressBuffer(first ? bytesA : bytesB);
    let writeBytes0 = __UnsizedRWByteAddressBuffer(first ? writtenBytesA : writtenBytesB);
    write0[index] = read0[0] + readBytes0.Load(0);
    writeBytes0.Store(index * 4, write0[index]);
    recordResult(index, 0);

    // Convert each resource before selecting the view.
    let read1 = first ? __UnsizedStructuredBuffer<uint>(valuesA)
                      : __UnsizedStructuredBuffer<uint>(valuesB);
    let write1 = first ? __UnsizedRWStructuredBuffer<uint>(writtenValuesA)
                       : __UnsizedRWStructuredBuffer<uint>(writtenValuesB);
    let readBytes1 = first ? __UnsizedByteAddressBuffer(bytesA) : __UnsizedByteAddressBuffer(bytesB);
    let writeBytes1 = first ? __UnsizedRWByteAddressBuffer(writtenBytesA)
                            : __UnsizedRWByteAddressBuffer(writtenBytesB);
    write1[index] = read1[0] + readBytes1.Load(0) + 1;
    writeBytes1.Store(index * 4, write1[index]);
    recordResult(index, 1);

    // Return the selected views through helpers before accessing the resources.
    let read2 = chooseValues(first);
    let write2 = chooseWrittenValues(first);
    let readBytes2 = chooseBytes(first);
    let writeBytes2 = chooseWrittenBytes(first);
    write2[index] = read2[0] + readBytes2.Load(0) + 2;
    writeBytes2.Store(index * 4, write2[index]);
    recordResult(index, 2);
}

// HLSL: void computeMain
// SPIRV-NOT: OpCapability VariablePointers
// SPIRV: OpEntryPoint GLCompute
// GLSL: void main()
// METAL: void computeMain
// METALLIB: define void @computeMain

// RESULT: 80
// RESULT-NEXT: 82
// RESULT-NEXT: 84
// RESULT-NEXT: 1200
// RESULT-NEXT: 1220
// RESULT-NEXT: 1240
