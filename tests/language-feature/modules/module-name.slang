//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK): -output-using-type

// Test that decls with the same name as the module are allowed.

module MyModule;

struct MyModule
{
    int doThing()
    {
        return 1;
    }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<int> output;

void computeMain()
{
    MyModule m;
    output[0] = m.doThing();
    // CHECK: 1
}
