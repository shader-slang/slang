//TEST:LANG_SERVER:
// Test case for public __init (should work)

struct TestStruct
{
    float value;
    
    // Public constructor - this should work fine  
    public __init(float val)
    {
        this.value = val;
    }
}

void test()
{
//HOVER:18,15
    TestStruct obj = TestStruct(1.0f);
}