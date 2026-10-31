//TEST:SIMPLE_EX(filecheck=CHECK): -lang hlsl tests/spirv/debug-lexical-scope-hlsl-for.slang -entry main -stage compute -target spirv -g2 -O0 -validate-ir
//TEST:SIMPLE_EX(filecheck=CHECK): tests/spirv/debug-lexical-scope-hlsl-for.slang -entry main -stage compute -target spirv -g2 -O0 -validate-ir

// An HLSL `for` loop scopes its initializer variable to the loop, as Slang does, so both
// languages give the loop a lexical block that holds the loop's `i` and contains the body's
// block, while the `i` declared before the loop stays in the function's scope (#14099). The
// last scope before the store after the loop is the function's, where that outer `i` lives.
// CHECK-DAG: [[NAME:%[0-9]+]] = OpString "main"
// CHECK-DAG: [[INDEX:%[0-9]+]] = OpString "i"
// CHECK-DAG: [[FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[NAME]]
// CHECK-DAG: [[FOR:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE:%[A-Za-z_0-9]+]] %uint_102 %uint_5 [[FUNC]]
// CHECK-DAG: [[BODY:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_104 %uint_9 [[FOR]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[INDEX]] {{.*}} [[SOURCE]] %uint_101 {{%uint_[0-9]+}} [[FUNC]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[INDEX]] {{.*}} [[SOURCE]] %uint_102 {{%uint_[0-9]+}} [[FOR]]
// CHECK: OpLoopMerge [[MERGE:%[A-Za-z_0-9]+]]
// CHECK: DebugScope [[BODY]]{{$}}
// CHECK: OpStore
// CHECK: [[MERGE]] = OpLabel
// CHECK-NOT: DebugScope [[BODY]]
// CHECK: DebugScope [[FOR]]{{$}}
// CHECK-NEXT: DebugScope [[FUNC]]{{$}}
// CHECK-NOT: DebugScope
// CHECK-NOT: DebugNoScope
// CHECK: OpStore

RWStructuredBuffer<int> outputBuffer;
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
#line 100 "hlsl-for.hlsl"
{
    int i = int(tid.y);
    for (int i = 0; i < int(tid.x); ++i)
    {
        outputBuffer[i] = i;
    }
    outputBuffer[0] = i;
}
