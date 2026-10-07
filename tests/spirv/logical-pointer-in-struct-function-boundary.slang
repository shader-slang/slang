//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O0
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O1
//TEST:SIMPLE(filecheck=PARAM):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -O0
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

// A struct holding a logical-storage-class pointer crosses function boundaries: it is returned by
// value, passed `inout`, and passed by value to functions that are not inlined. Type legalization
// splits the pointer out of the struct at each boundary, the same way it splits out a resource,
// so each function takes or returns the pointer separately.
// https://github.com/shader-slang/slang/issues/13206

struct View
{
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
    float tag;
}

// A struct holding both a resource and a logical pointer splits both out.
struct Mixed
{
    RWStructuredBuffer<float> buffer;
    Ptr<float, Access.ReadWrite, AddressSpace.GroupShared> data;
    float tag;
}

groupshared float gs[4];

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// CHECK-NOT: OpTypeStruct {{.*}}%_ptr_Workgroup
// CHECK-NOT: OpCompositeExtract %_ptr_Workgroup

// PARAM: OpFunctionParameter %_ptr_Workgroup_float

[noinline]
View makeView(int i)
{
    View v;
    v.data = __getAddress(gs[i]);
    v.tag = float(i);
    return v;
}

[noinline]
void bump(inout View v)
{
    v.data[0] = v.data[0] + 1.0f;
    v.tag = v.tag + 1.0f;
}

[noinline]
float readView(View v)
{
    return v.data[0] + v.tag;
}

[noinline]
void writeMixed(Mixed m)
{
    m.buffer[0] = m.data[0] + m.tag;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    View v = makeView(1);
    v.data[0] = 3.0f;
    bump(v);
    outputBuffer[1] = readView(v);

    gs[2] = 5.0f;
    Mixed m;
    m.buffer = outputBuffer;
    m.data = __getAddress(gs[2]);
    m.tag = 2.0f;
    writeMixed(m);

    // Element 0 holds `m.data[0] + m.tag` from `writeMixed`, and element 1 holds `readView(v)`.
    // BUF: 7
    // BUF-NEXT: 6
}
