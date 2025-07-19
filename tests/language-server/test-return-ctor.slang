//TEST:LANG_SERVER(filecheck=CHECK):
// Test in function return context

struct TestType
{
    __init(int x) {}
}

TestType createTest(int val)
{
//HOVER:13,18
    return TestType(val);
}

//CHECK: TestType.init