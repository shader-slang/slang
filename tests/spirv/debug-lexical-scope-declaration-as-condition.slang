//TEST:SIMPLE_EX(filecheck=CHECK): -lang hlsl tests/spirv/debug-lexical-scope-declaration-as-condition.slang -entry computeMain -stage compute -target spirv -g2 -O0 -validate-ir

// Each statement's scope contains the variable declared in its condition and encloses its
// body scopes. The local declared after the loop belongs to the function. The `#line`
// directives fix the line numbers for the scope checks and the initializer call.
// CHECK-DAG: OpName [[CHOOSE_THEN:%[A-Za-z_0-9]+]] "chooseThen"
// CHECK-DAG: [[MAIN_NAME:%[0-9]+]] = OpString "computeMain"
// CHECK-DAG: [[SELECTED_NAME:%[0-9]+]] = OpString "selected"
// CHECK-DAG: [[THEN_NAME:%[0-9]+]] = OpString "thenValue"
// CHECK-DAG: [[ELSE_NAME:%[0-9]+]] = OpString "elseValue"
// CHECK-DAG: [[REPEAT_NAME:%[0-9]+]] = OpString "repeat"
// CHECK-DAG: [[LOOP_NAME:%[0-9]+]] = OpString "loopValue"
// CHECK-DAG: [[AFTER_NAME:%[0-9]+]] = OpString "afterLoop"
// CHECK-DAG: [[MAIN_FUNC:%[A-Za-z_0-9]+]] = {{.*}} DebugFunction [[MAIN_NAME]]
// CHECK-DAG: [[IF_SCOPE:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE:%[A-Za-z_0-9]+]] %uint_101 {{%uint_[0-9]+}} [[MAIN_FUNC]]
// CHECK-DAG: [[THEN_SCOPE:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_103 {{%uint_[0-9]+}} [[IF_SCOPE]]
// CHECK-DAG: [[ELSE_SCOPE:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_108 {{%uint_[0-9]+}} [[IF_SCOPE]]
// CHECK-DAG: [[WHILE_SCOPE:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] %uint_200 {{%uint_[0-9]+}} [[MAIN_FUNC]]
// CHECK-DAG: [[LOOP_BODY:%[A-Za-z_0-9]+]] = {{.*}} DebugLexicalBlock [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[WHILE_SCOPE]]
// CHECK-DAG: DebugLocalVariable [[SELECTED_NAME]] {{.*}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[IF_SCOPE]]
// CHECK-DAG: DebugLocalVariable [[THEN_NAME]] {{.*}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[THEN_SCOPE]]
// CHECK-DAG: DebugLocalVariable [[ELSE_NAME]] {{.*}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[ELSE_SCOPE]]
// CHECK-DAG: DebugLocalVariable [[REPEAT_NAME]] {{.*}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[WHILE_SCOPE]]
// CHECK-DAG: DebugLocalVariable [[LOOP_NAME]] {{.*}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[LOOP_BODY]]
// CHECK-DAG: DebugLocalVariable [[AFTER_NAME]] {{.*}} [[SOURCE]] {{%uint_[0-9]+}} {{%uint_[0-9]+}} [[MAIN_FUNC]]

// The initializer call is on the declaration line. The `if` keyword is on the previous line.
// CHECK: DebugLine [[SOURCE]] %uint_101 %uint_101
// CHECK-NOT: DebugLine [[SOURCE]] %uint_100
// CHECK: OpFunctionCall %bool [[CHOOSE_THEN]]

RWStructuredBuffer<int> result;

[noinline]
bool chooseThen(uint value)
{
    return value > 0;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
#line 100
    if (
        bool selected = chooseThen(tid.x))
    {
        int thenValue = int(selected) + int(tid.x);
        result[0] = thenValue;
    }
    else
    {
        int elseValue = int(selected) + 1;
        result[0] = elseValue;
    }

    uint remaining = tid.x & 1;
#line 200
    while (bool repeat = remaining != 0)
    {
        int loopValue = int(repeat) + int(remaining);
        result[1] = loopValue;
        --remaining;
    }

    int afterLoop = int(tid.x) + 3;
    result[2] = afterLoop;
}
