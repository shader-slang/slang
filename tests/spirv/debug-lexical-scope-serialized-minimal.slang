//TEST:COMPILE: tests/spirv/debug-lexical-scope-serialized-minimal.slang -g1 -o tests/spirv/debug-lexical-scope-serialized-minimal.slang-module
//TEST:SIMPLE_EX(filecheck=FULL): tests/spirv/debug-lexical-scope-serialized-minimal.slang-module -entry main -stage compute -target spirv -g2 -O0 -validate-ir
//TEST:SIMPLE_EX(filecheck=FULL): tests/spirv/debug-lexical-scope-serialized-minimal.slang-module -entry main -stage compute -target spirv -g3 -O0 -validate-ir
//TEST:SIMPLE_EX(filecheck=MINIMAL): tests/spirv/debug-lexical-scope-serialized-minimal.slang-module -entry main -stage compute -target spirv -g1 -O0 -validate-ir

// Compile the serialized module directly: a new source file compiled at -g2 would
// supply a compilation unit and hide missing ownership in the -g1 module.
// Minimal IR must retain enough scope metadata to emit the available inline info
// at a higher debug level, without emitting extended debug instructions at -g1.
// FULL-DAG: [[MAIN_NAME:%[0-9]+]] = OpString "main"
// FULL-DAG: [[LEAF_NAME:%[0-9]+]] = OpString "leaf"
// FULL-DAG: [[CU:%[A-Za-z_0-9]+]] = {{.*}} DebugCompilationUnit
// FULL-DAG: [[MAIN:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[MAIN_NAME]] {{.*}} [[CU]]
// FULL-DAG: [[LEAF:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[LEAF_NAME]] {{.*}} [[CU]]
// FULL-DAG: [[SITE:%[A-Za-z_0-9]+]] = {{.*}} DebugInlinedAt {{%uint_[0-9]+}} [[MAIN]]{{$}}
// FULL: = OpFunction
// FULL: DebugScope [[LEAF]] [[SITE]]{{$}}
// FULL-NOT: Debug{{(No)?Scope}}
// FULL: OpIAdd
// MINIMAL-NOT: NonSemantic.Shader.DebugInfo
// MINIMAL: OpLine
// MINIMAL-NOT: NonSemantic.Shader.DebugInfo

RWStructuredBuffer<int> outputBuffer;
[ForceInline]
int leaf(int x)
{
    return x + 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = leaf(int(tid.x));
}
