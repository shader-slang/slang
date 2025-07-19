//TEST:LANG_SERVER:
// Test in function return context with public constructor

struct TestType
{
    public __init(int x) {}
}

TestType createTest(int val)
{
//HOVER:13,18
    return TestType(val);
}