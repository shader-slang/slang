//TEST:LANG_SERVER:
// Test case - step by step debugging with public constructor

struct TestStruct
{
    float value;
    
    // Public constructor
    public __init(float val)
    {
        this.value = val;
    }
}

void test()
{
//HOVER:18,5
    let obj = TestStruct(1.0f);
}