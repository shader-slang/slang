//TEST:SIMPLE_EX(filecheck=CHECK): -lang hlsl tests/spirv/debug-lexical-scope-unscoped-for.slang -entry main -stage compute -target spirv -g2 -O0 -validate-ir

// HLSL keeps the initializer in the enclosing scope, so i remains visible after
// the loop. Only the body braces introduce a lexical block.
// CHECK-DAG: [[NAME:%[0-9]+]] = OpString "main"
// CHECK-DAG: [[INDEX:%[0-9]+]] = OpString "i"
// CHECK-DAG: [[FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[NAME]]
// CHECK-DAG: [[BODY:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE:%[A-Za-z_0-9]+]] %uint_104 %uint_9 [[FUNC]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[INDEX]] {{.*}} [[SOURCE]] %uint_102 {{%uint_[0-9]+}} [[FUNC]]
// CHECK: OpLoopMerge [[MERGE:%[A-Za-z_0-9]+]]
// CHECK: DebugScope [[BODY]]{{$}}
// CHECK: OpStore
// CHECK: [[MERGE]] = OpLabel
// CHECK: DebugScope [[FUNC]]{{$}}
// CHECK-NOT: DebugNoScope
// CHECK-NOT: DebugScope [[BODY]]
// CHECK: OpStore

RWStructuredBuffer<int> outputBuffer;
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
#line 100 "unscoped-for.hlsl"
{
    // The loop initializer belongs to main, not a separate loop scope.
    for (int i = 0; i < int(tid.x); ++i)
    {
        outputBuffer[i] = i;
    }
    outputBuffer[0] = i;
}
