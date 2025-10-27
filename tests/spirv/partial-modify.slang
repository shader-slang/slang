//TEST:SIMPLE(filecheck=CHECK): -target spirv -O0

// Test that calling a function and passing an access-chain as argument result in legal SPIR-V
// that introduces temp variables as needed.

struct Sub
{
    int x;
}
struct Root
{
    Sub sub1;
    Sub sub2;
}

void modify(inout Sub s)
{
    s.x = 1;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    Root root;

    // SPIR-V has a debatable rule that OpFunctionCall arguments must be a direct reference to
    // a local/global variable, instead of an access chain. So we need to make sure that for the
    // call to `modify`, where we pass `root.sub1` and `root.sub2` (which are access chains),
    // we introduce a temp variable `tmp`, copy `root.sub1` into `tmp`, pass `tmp` to the function,
    // and then copy `tmp` back to `root.sub1`. The additional copy is unfortunate, but it is the only
    // way to make the SPIR-V legal without inlining the call.
    
    // CHECK: %[[AC:[A-Za-z0-9_]+]] = OpAccessChain %_ptr_Function_Sub
    // CHECK: %[[LOAD_TMP:[A-Za-z0-9_]+]] = OpLoad %Sub %[[AC]]
    // CHECK: OpStore %[[TMP:[A-Za-z0-9_]+]] %[[LOAD_TMP]]
    // CHECK: OpFunctionCall %void %modify %[[TMP]]
    // CHECK: %[[LOAD_TMP2:[A-Za-z0-9_]+]] = OpLoad %Sub %[[TMP]]
    // CHECK: OpStore %[[AC]] %[[LOAD_TMP2]]

    modify(root.sub1);
    modify(root.sub2);

    outputBuffer[0] = root.sub1.x + root.sub2.x;
}