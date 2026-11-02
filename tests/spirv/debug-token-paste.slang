//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0 -validate-ir -preserve-embedded-source
//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O1 -validate-ir -preserve-embedded-source
//TEST:SIMPLE(filecheck=LOW): -target spirv -g1 -O0 -validate-ir
//TEST:SIMPLE(filecheck=LOW): -target spirv -g0 -O0 -validate-ir
//TEST:SIMPLE(filecheck=CUDA): -target cuda -g2

// Token-pasted declaration names have synthetic source locations. Debug metadata
// must use the originating source for both the function scope and local variables.
// Keep the macro source distinct from its caller, including its remapped line numbers.
#line 100 "token-paste-macros.slang"
#define TEST_CASE(name) [noinline] int test_##name(int input)
#define LOCAL(name, value) let local_##name = value
#define JOIN_INNER(a, b) a##b
#define JOIN(a, b) JOIN_INNER(a, b)
#define NESTED_TEST(name) [noinline] int JOIN(nested_, name)(int input)

#line 200 "token-paste-caller.slang"
TEST_CASE(value)
{
    let value = input + 1;
    LOCAL(pasted, value + 1);
    return local_pasted;
}

NESTED_TEST(value)
{
    let nestedValue = input + 2;
    return nestedValue;
}

RWStructuredBuffer<int> outputBuffer;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = test_value(int(tid.x)) + nested_value(int(tid.x));
}

// CHECK-DAG: [[MACRO_FILE:%[0-9]+]] = OpString "{{.*}}token-paste-macros.slang"
// CHECK-DAG: [[MACRO_SOURCE:%[0-9]+]] = {{.*}} DebugSource [[MACRO_FILE]]
// CHECK-DAG: [[CALLER_FILE:%[0-9]+]] = OpString "{{.*}}token-paste-caller.slang"
// CHECK-DAG: [[CALLER_SOURCE:%[0-9]+]] = {{.*}} DebugSource [[CALLER_FILE]]
// CHECK-DAG: [[FUNC_NAME:%[0-9]+]] = OpString "test_value"
// CHECK-DAG: [[NESTED_NAME:%[0-9]+]] = OpString "nested_value"
// CHECK-DAG: [[VALUE_NAME:%[0-9]+]] = OpString "value"
// CHECK-DAG: [[PASTED_NAME:%[0-9]+]] = OpString "local_pasted"
// CHECK-DAG: [[NESTED_VALUE_NAME:%[0-9]+]] = OpString "nestedValue"
// CHECK-DAG: [[FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[FUNC_NAME]] {{.*}} [[MACRO_SOURCE]] %uint_100
// CHECK-DAG: [[NESTED_FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[NESTED_NAME]] {{.*}} [[MACRO_SOURCE]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[VALUE_NAME]] {{.*}} [[CALLER_SOURCE]] %uint_202 {{%uint_[0-9]+}} [[FUNC]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[PASTED_NAME]] {{.*}} [[MACRO_SOURCE]] %uint_101 {{%uint_[0-9]+}} [[FUNC]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[NESTED_VALUE_NAME]] {{.*}} [[CALLER_SOURCE]] %uint_209 {{%uint_[0-9]+}} [[NESTED_FUNC]]

// LOW: OpEntryPoint GLCompute
// CUDA: static __device__ __noinline__ int test_value
// CUDA: static __device__ __noinline__ int nested_value
