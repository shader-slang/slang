//TEST:LANG_SERVER(filecheck=CHECK):
struct TestType
{
    public __init(int x) {}
}

void test()
{
//HOVER:10,18
    let obj = TestType(5);
}

//CHECK: TestType.init