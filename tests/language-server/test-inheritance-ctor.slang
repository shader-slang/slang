//TEST:LANG_SERVER(filecheck=CHECK):
// Simple version to test if inheritance affects it

interface ITest
{
    float getValue();
}

struct TestType : ITest
{
    __init(int x) {}
    float getValue() { return 1.0f; }
}

void test()
{
//HOVER:18,18
    let obj = TestType(5);
}

//CHECK: TestType.init