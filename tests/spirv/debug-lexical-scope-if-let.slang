//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0 -validate-ir

// CHECK-DAG: [[SOURCE_FILE:%[0-9]+]] = OpString "{{.*}}debug-lexical-scope-if-let.slang"
// CHECK-DAG: [[SOURCE:%[0-9]+]] = {{.*}} DebugSource [[SOURCE_FILE]]
// CHECK-DAG: [[MAIN_NAME:%[0-9]+]] = OpString "computeMain"
// CHECK-DAG: [[ITEM_NAME:%[0-9]+]] = OpString "item"
// CHECK-DAG: [[MAIN_FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[MAIN_NAME]] {{.*}} [[SOURCE]]
// CHECK-DAG: [[IFLET_BLOCK_A:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[MAIN_FUNC]]
// CHECK-DAG: [[IFLET_BODY_A:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[IFLET_BLOCK_A]]
// CHECK-DAG: [[IFLET_BLOCK_B:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[MAIN_FUNC]]
// CHECK-DAG: [[IFLET_BODY_B:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[IFLET_BLOCK_B]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[ITEM_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[IFLET_BLOCK_A]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[ITEM_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[IFLET_BLOCK_B]]
// CHECK-DAG: {{.*}} DebugScope [[IFLET_BLOCK_A]]
// CHECK-DAG: {{.*}} DebugScope [[IFLET_BODY_A]]
// CHECK-DAG: {{.*}} DebugScope [[IFLET_BLOCK_B]]
// CHECK-DAG: {{.*}} DebugScope [[IFLET_BODY_B]]

RWStructuredBuffer<int> result;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Optional<int> first = int(tid.x) + 1;
    Optional<int> second = int(tid.x) + 2;
    int value = 0;

    if (let item = first)
    {
        value += item;
    }

    if (let item = second)
    {
        value += item;
    }

    result[0] = value;
}
