// Debugger support is temporarily non-functional while debug-scope tracking is redesigned.
//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry main -g2

enum DebugError
{
    fail,
};

RWStructuredBuffer<int> output;

int mayFail(bool condition) throws DebugError
{
    if (condition)
        throw DebugError.fail;
    return 1;
}

[numthreads(1, 1, 1)]
void main()
{
    do
    {
        int value = try mayFail(true);
        output[0] = value;
    }
    catch(error: DebugError)
    {
        int caught = int(error);
        output[0] = caught;
    }
}

// CHECK-DAG: [[STR_MAIN:%[0-9]+]] = OpString "main"
// CHECK-DAG: [[STR_ERROR:%[0-9]+]] = OpString "error"
// CHECK-DAG: [[MAIN:%[0-9]+]] = OpExtInst %void {{.*}} DebugFunction [[STR_MAIN]]
// CHECK-DAG: [[BODY:%[0-9]+]] = OpExtInst %void {{.*}} DebugLexicalBlock {{.*}} [[MAIN]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} = OpExtInst %void {{.*}} DebugLocalVariable [[STR_ERROR]] {{.*}} [[CATCH:%[0-9]+]] %uint_0
// CHECK-DAG: [[CATCH]] = OpExtInst %void {{.*}} DebugLexicalBlock {{.*}} [[BODY]]
