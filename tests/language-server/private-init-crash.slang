//TEST:LANG_SERVER(filecheck=CHECK):
// Test case for crash when using private __init method

struct TestStruct
{
    float value;
    
    // Private constructor - this should not cause a crash
    __init(float val)
    {
        this.value = val;
    }
}

void test()
{
//HOVER:18,12
    let obj = TestStruct(1.0f);
}

//CHECK: TestStruct.init