//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -g2 -emit-spirv-directly -O0

//TEST:COMPILE: tests/spirv/debug-info-artificial-generics.slang -g2 -o tests/spirv/debug-info-artificial-generics.slang-module
//TEST:SIMPLE_EX(filecheck=CHECK): tests/spirv/debug-info-artificial-generics.slang-module -entry main -stage compute -target spirv-asm -g2 -O0 -validate-ir

// Check that specialization and module serialization preserve both constructors' artificial flags.
//CHECK-DAG: [[BOX:%[0-9]+]] = OpString "Box.$init"
//CHECK: [[ARTIFICIAL:%[a-zA-Z0-9_]+]] = OpConstant %uint 32
//CHECK-COUNT-2: DebugFunction [[BOX]] {{.*}} [[BOX]] [[ARTIFICIAL]]
//CHECK-NOT: DebugFunction [[BOX]]

struct Box<T>
{
    T value;
};

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Box<float> first = {float(tid.x)};
    Box<uint> second = {2u};
    outputBuffer[tid.x] = first.value + float(second.value);
}
