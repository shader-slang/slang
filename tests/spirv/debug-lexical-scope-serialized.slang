// Serializing an early-inlined nested block must preserve both its parent and inline chain.
//TEST:COMPILE: tests/spirv/debug-lexical-scope-module.slang -g3 -o tests/spirv/debug-lexical-scope-module.slang-module
//TEST:SIMPLE(filecheck=CHECK): -r tests/spirv/debug-lexical-scope-module.slang-module -target spirv -g3 -O0 -validate-ir

import "debug-lexical-scope-module";
RWStructuredBuffer<int> outputBuffer;
[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    {
        int value = importedLexicalOuter(int(tid.x));
        outputBuffer[0] = value;
    }
    outputBuffer[1] = 123;
}

// CHECK-DAG: [[MAIN_NAME:%[0-9]+]] = OpString "main"
// CHECK-DAG: [[OUTER_NAME:%[0-9]+]] = OpString "importedLexicalOuter"
// CHECK-DAG: [[LEAF_NAME:%[0-9]+]] = OpString "lexicalLeaf"
// CHECK-DAG: [[MAIN:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[MAIN_NAME]]
// CHECK-DAG: [[OUTER:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[OUTER_NAME]]
// CHECK-DAG: [[LEAF:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[LEAF_NAME]]
// CHECK-DAG: [[MAIN_BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock {{.*}} [[MAIN]]
// CHECK-DAG: [[OUTER_BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock {{.*}} [[OUTER]]
// CHECK-DAG: [[LEAF_BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock {{.*}} [[LEAF]]
// CHECK-DAG: [[OUTER_SITE:%[A-Za-z_0-9]+]] = {{.*}} DebugInlinedAt {{%uint_[0-9]+}} [[MAIN_BLOCK]]
// CHECK-DAG: [[LEAF_SITE:%[A-Za-z_0-9]+]] = {{.*}} DebugInlinedAt {{%uint_[0-9]+}} [[OUTER_BLOCK]] [[OUTER_SITE]]
// CHECK: DebugScope [[LEAF_BLOCK]] [[LEAF_SITE]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: [[LEAF_RESULT:%[A-Za-z_0-9]+]] = OpIAdd %int {{%[A-Za-z_0-9]+}} %int_1
// CHECK: DebugScope [[OUTER_BLOCK]] [[OUTER_SITE]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpIAdd %int [[LEAF_RESULT]] %int_2
// CHECK: DebugScope [[MAIN_BLOCK]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore
// CHECK: DebugScope [[MAIN]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpStore {{.*}} %int_123
