//TEST:LANG_SERVER(filecheck=CHECK):
struct MyPrivateCtorType
{
    __init(int x) {}
}

void test()
{
//HOVER:10,23
    let obj = MyPrivateCtorType(5);
}

//CHECK: MyPrivateCtorType.init