//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0 -validate-ir
//TEST:SIMPLE(filecheck=CHECK_OPT): -target spirv -g2 -O1
//TEST:SIMPLE(filecheck=CHECK_G1): -target spirv -g1 -O0

// CHECK-DAG: [[SOURCE_FILE:%[0-9]+]] = OpString "{{.*}}debug-lexical-scope.slang"
// CHECK-DAG: [[SOURCE:%[0-9]+]] = {{.*}} DebugSource [[SOURCE_FILE]]
// CHECK-DAG: [[MAIN_NAME:%[0-9]+]] = OpString "computeMain"
// CHECK-DAG: [[HELPER_NAME:%[0-9]+]] = OpString "inlineHelper"
// CHECK-DAG: [[NESTED_NAME:%[0-9]+]] = OpString "nestedInline"
// CHECK-DAG: [[GENERIC_NAME:%[0-9]+]] = OpString "genericBlock"
// CHECK-DAG: [[GENERIC_INLINE_NAME:%[0-9]+]] = OpString "genericInline"
// CHECK-DAG: [[SHADOW_STR:%[0-9]+]] = OpString "shadow"
// CHECK-DAG: [[INNER_VALUE_STR:%[0-9]+]] = OpString "innerValue"
// CHECK-DAG: [[BRANCH_STR:%[0-9]+]] = OpString "branchValue"
// CHECK-DAG: [[FOR_INDEX_STR:%[0-9]+]] = OpString "i"
// CHECK-DAG: [[LOOP_STR:%[0-9]+]] = OpString "loopValue"
// CHECK-DAG: [[WHILE_STR:%[0-9]+]] = OpString "whileValue"
// CHECK-DAG: [[INLINED_STR:%[0-9]+]] = OpString "inlinedValue"
// CHECK-DAG: [[INLINED_BRANCH_STR:%[0-9]+]] = OpString "inlinedBranchValue"
// CHECK-DAG: [[NESTED_VALUE_STR:%[0-9]+]] = OpString "nestedValue"
// CHECK-DAG: [[GENERIC_VALUE_STR:%[0-9]+]] = OpString "genericValue"
// CHECK-DAG: [[GENERIC_INLINE_VALUE_STR:%[0-9]+]] = OpString "genericInlineValue"

// CHECK-DAG: [[MAIN_FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[MAIN_NAME]] {{.*}} [[SOURCE]]
// CHECK-DAG: [[HELPER_FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[HELPER_NAME]] {{.*}} [[SOURCE]]
// CHECK-DAG: [[NESTED_FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[NESTED_NAME]] {{.*}} [[SOURCE]]
// CHECK-DAG: [[GENERIC_FUNC_A:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[GENERIC_NAME]] {{.*}} [[SOURCE]]
// CHECK-DAG: [[GENERIC_FUNC_B:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[GENERIC_NAME]] {{.*}} [[SOURCE]]
// CHECK-DAG: [[GENERIC_INLINE_FUNC_A:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[GENERIC_INLINE_NAME]] {{.*}} [[SOURCE]]
// CHECK-DAG: [[GENERIC_INLINE_FUNC_B:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[GENERIC_INLINE_NAME]] {{.*}} [[SOURCE]]
// Exact line/column operands below pin each lexical block to the intended source scope.
// CHECK-DAG: [[OUTER_BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_133 %uint_9 [[MAIN_FUNC]]
// CHECK-DAG: [[INNER_BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_135 %uint_13 [[OUTER_BLOCK]]
// CHECK-DAG: [[BRANCH_BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_144 %uint_9 [[MAIN_FUNC]]
// CHECK-DAG: [[FOR_BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_148 %uint_5 [[MAIN_FUNC]]
// CHECK-DAG: [[LOOP_BODY_BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_150 %uint_9 [[FOR_BLOCK]]
// CHECK-DAG: [[WHILE_BODY_BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_156 %uint_9 [[MAIN_FUNC]]
// CHECK-DAG: [[HELPER_BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_100 %uint_9 [[HELPER_FUNC]]
// CHECK-DAG: [[HELPER_BRANCH_BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_103 %uint_13 [[HELPER_BLOCK]]
// CHECK-DAG: [[NESTED_BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_91 %uint_9 [[NESTED_FUNC]]
// CHECK-DAG: [[GENERIC_BLOCK_A:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_113 %uint_9 [[GENERIC_FUNC_A]]
// CHECK-DAG: [[GENERIC_BLOCK_B:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_113 %uint_9 [[GENERIC_FUNC_B]]
// CHECK-DAG: [[GENERIC_INLINE_BLOCK_A:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_122 %uint_9 [[GENERIC_INLINE_FUNC_A]]
// CHECK-DAG: [[GENERIC_INLINE_BLOCK_B:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_122 %uint_9 [[GENERIC_INLINE_FUNC_B]]

// CHECK-DAG: {{.*}} DebugLocalVariable [[SHADOW_STR]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[OUTER_BLOCK]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[SHADOW_STR]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[INNER_BLOCK]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[INNER_VALUE_STR]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[INNER_BLOCK]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[BRANCH_STR]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[BRANCH_BLOCK]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[FOR_INDEX_STR]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[FOR_BLOCK]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[LOOP_STR]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[LOOP_BODY_BLOCK]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[WHILE_STR]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[WHILE_BODY_BLOCK]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[INLINED_STR]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[HELPER_BLOCK]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[INLINED_BRANCH_STR]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[HELPER_BRANCH_BLOCK]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[NESTED_VALUE_STR]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[NESTED_BLOCK]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[GENERIC_VALUE_STR]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[GENERIC_BLOCK_A]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[GENERIC_VALUE_STR]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[GENERIC_BLOCK_B]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[GENERIC_INLINE_VALUE_STR]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[GENERIC_INLINE_BLOCK_A]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[GENERIC_INLINE_VALUE_STR]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[GENERIC_INLINE_BLOCK_B]]

// CHECK-DAG: {{.*}} DebugScope [[OUTER_BLOCK]]
// CHECK-DAG: {{.*}} DebugScope [[INNER_BLOCK]]
// CHECK-DAG: {{.*}} DebugScope [[FOR_BLOCK]]
// CHECK-DAG: {{.*}} DebugScope [[LOOP_BODY_BLOCK]]
// CHECK-DAG: {{.*}} DebugScope [[WHILE_BODY_BLOCK]]
// CHECK-DAG: [[INLINE_AT:%[A-Za-z_0-9]+]] = {{.*}} DebugInlinedAt {{%uint_[0-9]+}} [[OUTER_BLOCK]]
// CHECK-DAG: {{.*}} DebugScope [[HELPER_BLOCK]] [[INLINE_AT]]
// CHECK-DAG: {{.*}} DebugScope [[HELPER_BRANCH_BLOCK]] [[INLINE_AT]]
// CHECK-DAG: [[NESTED_INLINE_AT:%[A-Za-z_0-9]+]] = {{.*}} DebugInlinedAt {{%uint_[0-9]+}} [[HELPER_BLOCK]] [[INLINE_AT]]
// CHECK-DAG: {{.*}} DebugScope [[NESTED_BLOCK]] [[NESTED_INLINE_AT]]
// CHECK-DAG: {{.*}} DebugScope [[GENERIC_INLINE_BLOCK_A]] {{%[A-Za-z_0-9]+}}
// CHECK-DAG: {{.*}} DebugScope [[GENERIC_INLINE_BLOCK_B]] {{%[A-Za-z_0-9]+}}

// CHECK_OPT-DAG: [[OPT_SOURCE_FILE:%[0-9]+]] = OpString "{{.*}}debug-lexical-scope.slang"
// CHECK_OPT-DAG: [[OPT_SOURCE:%[0-9]+]] = {{.*}} DebugSource [[OPT_SOURCE_FILE]]
// CHECK_OPT-DAG: [[OPT_MAIN_NAME:%[0-9]+]] = OpString "computeMain"
// CHECK_OPT-DAG: [[OPT_FOR_INDEX_STR:%[0-9]+]] = OpString "i"
// CHECK_OPT-DAG: [[OPT_MAIN_FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[OPT_MAIN_NAME]] {{.*}} [[OPT_SOURCE]]
// CHECK_OPT-DAG: [[OPT_FOR_BLOCK:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[OPT_SOURCE]] %uint_148 %uint_5 [[OPT_MAIN_FUNC]]
// CHECK_OPT-DAG: {{.*}} DebugLocalVariable [[OPT_FOR_INDEX_STR]] {{%[A-Za-z_0-9]+}} [[OPT_SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[OPT_FOR_BLOCK]]
// CHECK_OPT-DAG: {{.*}} DebugScope [[OPT_FOR_BLOCK]]

// CHECK_G1-NOT: DebugLexicalBlock

RWStructuredBuffer<int> result;

[ForceInline]
int nestedInline(int value)
{
    {
        int nestedValue = value + 5;
        return nestedValue;
    }
}

[ForceInline]
int inlineHelper(int value)
{
    {
        int inlinedValue = nestedInline(value) + 3;
        if (inlinedValue > 0)
        {
            int inlinedBranchValue = inlinedValue + 1;
            inlinedValue = inlinedBranchValue;
        }
        return inlinedValue;
    }
}

T genericBlock<T>(T value)
{
    {
        T genericValue = value;
        return genericValue;
    }
}

[ForceInline]
T genericInline<T>(T value)
{
    {
        T genericInlineValue = value;
        return genericInlineValue;
    }
}
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int value = int(tid.x);

    {
        int shadow = value + 1;
        {
            int shadow = value + 2;
            int innerValue = shadow + value;
            value += innerValue;
        }
        value += inlineHelper(shadow);
    }

    if (value > 0)
    {
        int branchValue = value + 4;
        value += branchValue;
    }

    for (int i = 0; i < 1; ++i)
    {
        int loopValue = value + i;
        value += loopValue;
    }

    while (value < 1000)
    {
        int whileValue = value + 7;
        value += whileValue;
    }

    value += genericBlock<int>(value);
    value += int(genericBlock<uint>(uint(value)));
    value += genericInline<int>(value);
    value += int(genericInline<uint>(uint(value)));
    value += genericInlineSameSignature<int>(value);
    value += genericInlineSameSignature<uint>(value);

    result[0] = value;
}

[ForceInline]
int genericInlineSameSignature<T>(int value)
{
    {
        T genericSameValue;
        return value + 1;
    }
}

// CHECK-DAG: [[GENERIC_SAME_NAME:%[0-9]+]] = OpString "genericInlineSameSignature"
// CHECK-DAG: [[GENERIC_SAME_VALUE_STR:%[0-9]+]] = OpString "genericSameValue"
// CHECK-DAG: [[GENERIC_SAME_FUNC_A:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[GENERIC_SAME_NAME]] {{.*}} [[SOURCE]]
// CHECK-DAG: [[GENERIC_SAME_FUNC_B:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[GENERIC_SAME_NAME]] {{.*}} [[SOURCE]]
// CHECK-DAG: [[GENERIC_SAME_BLOCK_A:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[GENERIC_SAME_FUNC_A]]
// CHECK-DAG: [[GENERIC_SAME_BLOCK_B:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[GENERIC_SAME_FUNC_B]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[GENERIC_SAME_VALUE_STR]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[GENERIC_SAME_BLOCK_A]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[GENERIC_SAME_VALUE_STR]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[GENERIC_SAME_BLOCK_B]]
// CHECK-DAG: {{.*}} DebugScope [[GENERIC_SAME_BLOCK_A]] {{%[A-Za-z_0-9]+}}
// CHECK-DAG: {{.*}} DebugScope [[GENERIC_SAME_BLOCK_B]] {{%[A-Za-z_0-9]+}}

// Parameters and outermost body locals belong directly to the function.
// CHECK-DAG: [[TID_NAME:%[0-9]+]] = OpString "tid"
// CHECK-DAG: [[VALUE_NAME:%[0-9]+]] = OpString "value"
// CHECK-DAG: {{.*}} DebugLocalVariable [[TID_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[MAIN_FUNC]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[VALUE_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[MAIN_FUNC]]
// CHECK-DAG: {{.*}} DebugLocalVariable [[VALUE_NAME]] {{%[A-Za-z_0-9]+}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[HELPER_FUNC]]
