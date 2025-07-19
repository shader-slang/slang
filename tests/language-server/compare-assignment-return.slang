//TEST:LANG_SERVER:
// Test assignment vs return

struct TestType
{
    __init(int x) {}
}

void test()
{
//HOVER:12,18
    let obj = TestType(5); // This works
}

TestType createTest()
{
//HOVER:19,18
    return TestType(5); // This fails
}