//TEST:LANG_SERVER:
// Test case - step by step debugging

struct TestStruct
{
    float value;
    
    // Private constructor
    __init(float val)
    {
        this.value = val;
    }
}

void test()
{
//HOVER:18,5
    let obj = TestStruct(1.0f);
}