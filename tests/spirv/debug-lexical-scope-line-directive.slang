//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0 -validate-ir -preserve-embedded-source

// CHECK-DAG: [[REMAP_FILE:%[0-9]+]] = OpString {{.*}}remapped-lexical-scope.slang"
// CHECK-DAG: [[REMAP_SOURCE:%[0-9]+]] = {{.*}} DebugSource [[REMAP_FILE]]
// CHECK-DAG: [[MAIN_NAME:%[0-9]+]] = OpString "computeMain"
// CHECK-DAG: [[VALUE_NAME:%[0-9]+]] = OpString "remappedValue"
// CHECK-DAG: [[MAIN_FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[MAIN_NAME]] {{.*}} [[REMAP_SOURCE]]
// CHECK-DAG: [[LEXICAL_BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[REMAP_SOURCE]] %uint_104 %uint_9 [[MAIN_FUNC]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[VALUE_NAME]] {{%[A-Za-z_0-9]+}} [[REMAP_SOURCE]] %uint_104 %uint_13 [[LEXICAL_BLOCK]]
// CHECK-DAG: {{.*}} DebugScope [[LEXICAL_BLOCK]]

RWStructuredBuffer<int> result;
[shader("compute")]
#line 99 "remapped-lexical-scope.slang"
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int value = int(tid.x);
    {
        int remappedValue = value + 1;
        value += remappedValue;
    }
    result[0] = value;
}
