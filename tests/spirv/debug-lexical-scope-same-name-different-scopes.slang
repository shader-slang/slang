// clang-format off
//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0

// CHECK-DAG: [[SOURCE_FILE:%[0-9]+]] = OpString "{{.*}}debug-lexical-scope-same-name-different-scopes.slang"
// CHECK-DAG: [[SOURCE:%[0-9]+]] = {{.*}} DebugSource [[SOURCE_FILE]]
// CHECK-DAG: [[MAIN_NAME:%[0-9]+]] = OpString "main"
// CHECK-DAG: [[SCALAR_NAME:%[0-9]+]] = OpString "scalar"
// CHECK-DAG: [[VECTOR_NAME:%[0-9]+]] = OpString "vector"
// CHECK-DAG: [[MAIN_FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[MAIN_NAME]] {{.*}} [[SOURCE]]
// CHECK-DAG: [[IF_BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[MAIN_FUNC]]
// CHECK-DAG: [[BARE_BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[MAIN_FUNC]]

// CHECK-DAG: {{.*}} DebugLocalVariable [[SCALAR_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[MAIN_FUNC]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[SCALAR_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[IF_BLOCK]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[SCALAR_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[BARE_BLOCK]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[VECTOR_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[MAIN_FUNC]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[VECTOR_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[IF_BLOCK]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[VECTOR_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[BARE_BLOCK]]
// CHECK-DAG: {{.*}} DebugScope [[IF_BLOCK]]
// CHECK-DAG: {{.*}} DebugScope [[BARE_BLOCK]]

// clang-format on

// Each scalar/vector pair belongs to its declaration scope, including after
// leaving the two inner scopes. Store every pair so the values stay observable.
RWStructuredBuffer<int2> results;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int scalar = int(tid.x);
    int2 vector = int2(scalar, scalar + 1);
    results[0] = vector + scalar;

    if (tid.y != 0)
    {
        int scalar = int(tid.y) + 2;
        int2 vector = int2(scalar + 3, scalar + 4);
        results[1] = vector + scalar;
    }

    results[2] = vector + scalar;
    {
        int scalar = int(tid.z) + 5;
        int2 vector = int2(scalar + 6, scalar + 7);
        results[3] = vector + scalar;
    }
    results[4] = vector + scalar;
}
