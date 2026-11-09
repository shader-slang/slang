//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default
//TEST:INTERPRET(hgvm,filecheck=CHECK): -api wgpu

// A captured aggregate following a uint3 needs padding in a WGSL uniform buffer.
// The RHI session must use the same layout for generated code and reflection.
[shader("compute")]
[numthreads(1, 1, 1)]
void invoke<Func : IFunc<void, uint>>(
    uniform uint3 extent,
    uniform uint3 groups,
    uniform Func callback)
{
    callback(extent.x + groups.x);
}

[host]
void launch<Func : IFunc<void, uint>>(Func callback)
{
    dispatch(invoke<Func>, uint3(1, 1, 1))(
        uint3(5, 1, 1), uint3(3, 1, 1), callback);
}

[host]
void main()
{
    let output = RWStructuredBuffer<float>::alloc(1);
    float scale = 2.0;
    launch((uint index) => { output[0] = scale * float(index) + 1.0; });
    print(readBack(output, 0));
}

// CHECK: 17.000000
