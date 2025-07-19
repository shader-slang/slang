//TEST:LANG_SERVER(filecheck=CHECK):
// Test case showing that private constructors now work in assignment context
// This demonstrates the fix to maybeRedirectToConstructor

struct TestStructWithPrivateCtor
{
    float value;
    
    // Private constructor - should still show constructor info with the fix
    __init(float val)
    {
        this.value = val;
    }
}

void test()
{
//HOVER:18,25
    let obj = TestStructWithPrivateCtor(1.0f);
}

//CHECK: TestStructWithPrivateCtor.init