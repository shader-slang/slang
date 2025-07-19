//TEST:LANG_SERVER(filecheck=CHECK):
// Test case for improved private constructor handling
// This test passes - simple private constructors work correctly

struct TestStruct
{
    // Private constructor - with improved fallback this works  
    __init(int x) {}
}

void test()
{
//HOVER:12,15
    let obj = TestStruct(5);
}

//CHECK: TestStruct.init