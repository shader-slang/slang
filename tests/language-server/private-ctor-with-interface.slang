//TEST:LANG_SERVER(filecheck=CHECK):
interface ITestInterface
{
    float getValue();
}

struct MyPrivateCtorType : ITestInterface
{
    __init(int x) {}
    float getValue() { return 1.0f; }
}

void test()
{
//HOVER:14,23
    let obj = MyPrivateCtorType(5);
}

//CHECK: MyPrivateCtorType.init