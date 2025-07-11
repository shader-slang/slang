//TEST:LANG_SERVER(filecheck=CHECK):
struct MyType
{
    __init(int x) {}
}

void test()
{
//HOVER:10,18
    let obj = MyType(5);
    //        ^^^^^
    //        Hover here should show info for the ctor, not the type.
}

//CHECK: MyType.init
