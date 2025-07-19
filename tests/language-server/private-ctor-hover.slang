//TEST:LANG_SERVER(filecheck=CHECK):
struct MyPrivateType
{
    __init(int x) {}
}

void test()
{
//HOVER:10,18
    let obj = MyPrivateType(5);
    //        ^^^^^^^^^
    //        Hover here should show info for the private ctor.
}

//CHECK: MyPrivateType.init
