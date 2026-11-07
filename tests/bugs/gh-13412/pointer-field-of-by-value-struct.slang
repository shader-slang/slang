// A store back through a pointer field of a struct passed by value must not be removed across a
// call that writes the same memory through another copy of that pointer (shader-slang/slang#13412).
// The address `p.data[i]` has a `getOffsetPtr` root inside `saveAndRestore`, which is not storage
// private to the function.
// The CPU lane passes `-g0`: render-test compiles with debug info by default, and that keeps the
// store the removal would delete, so the lane would pass even on a miscompiling compiler.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -output-using-type -g0
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -entry computeMain -stage compute -O0
//TEST:SIMPLE(filecheck=SPV3): -target spirv-asm -entry computeMain -stage compute -O3

struct Pointers
{
    float2* other;
    float2* data;
};

//TEST_INPUT: set cb = new Pointers{ ubuffer(data=[9.0 9.0 9.0 9.0 9.0 9.0 9.0 9.0], stride=8), ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0], stride=8) }
ConstantBuffer<Pointers> cb;

//TEST_INPUT: set outb = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outb;

void overwrite(Pointers p, uint i)
{
    p.data[i] = p.other[i];
}

void saveAndRestore(Pointers p, uint i)
{
    const float2 saved = p.data[i];
    overwrite(p, i);
    p.data[i] = saved;
}

// SPV-LABEL: %saveAndRestore = OpFunction
// SPV: %saved = OpLoad %v2float [[ADDR:%[0-9]+]]
// SPV-NEXT: OpFunctionCall %void %overwrite
// SPV: OpStore {{%[0-9]+}} %saved
// SPV: OpFunctionEnd

// SPV3-LABEL: %computeMain = OpFunction
// SPV3: [[SAVED:%[0-9]+]] = OpLoad %v2float [[DATA:%[0-9]+]]
// SPV3: OpStore {{%[0-9]+}} {{%[0-9]+}}
// SPV3: OpStore [[DATA]] [[SAVED]]

[numthreads(4, 1, 1)]
void computeMain(uint3 id: SV_DispatchThreadID)
{
    saveAndRestore(cb, id.x);
    float2 v = cb.data[id.x];
    outb[2 * id.x] = v.x;
    outb[2 * id.x + 1] = v.y;
}

// BUF: type: float
// BUF-NEXT: {{^}}1.000000{{$}}
// BUF-NEXT: {{^}}2.000000{{$}}
// BUF-NEXT: {{^}}3.000000{{$}}
// BUF-NEXT: {{^}}4.000000{{$}}
// BUF-NEXT: {{^}}5.000000{{$}}
// BUF-NEXT: {{^}}6.000000{{$}}
// BUF-NEXT: {{^}}7.000000{{$}}
// BUF-NEXT: {{^}}8.000000{{$}}
