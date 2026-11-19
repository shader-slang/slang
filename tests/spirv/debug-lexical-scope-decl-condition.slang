//TEST:SIMPLE_EX(filecheck=CHECK): -target spirv -g2 -O0 -validate-ir -lang hlsl -stage compute -entry computeMain tests/spirv/debug-lexical-scope-decl-condition.slang

// A declaration condition introduces its own debug lexical block, covering the
// condition and the arms, and the declared variable's debug info points into that
// block (rather than the enclosing function), mirroring what the `if (let ...)`
// desugaring achieves through its wrapper block.

// CHECK-DAG: [[SOURCE_FILE:%[0-9]+]] = OpString "{{.*}}debug-lexical-scope-decl-condition.slang"
// CHECK-DAG: [[SOURCE:%[0-9]+]] = {{.*}} DebugSource [[SOURCE_FILE]]
// CHECK-DAG: [[MAIN_NAME:%[0-9]+]] = OpString "computeMain"
// CHECK-DAG: [[COUNT_NAME:%[0-9]+]] = OpString "count"
// CHECK-DAG: [[MAIN_FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[MAIN_NAME]] {{.*}} [[SOURCE]]
// CHECK-DAG: [[IF_COND_BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[MAIN_FUNC]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[COUNT_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[IF_COND_BLOCK]]
// CHECK-DAG: {{.*}} DebugScope [[IF_COND_BLOCK]]
// CHECK-DAG: {{.*}} DebugScope [[MAIN_FUNC]]

RWStructuredBuffer<int> result;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int c = 3;
    if (int count = c)
    {
        result[0] = count;
    }
    result[1] = c;
}
