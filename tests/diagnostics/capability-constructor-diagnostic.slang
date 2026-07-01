//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -emit-spirv-directly -entry main -stage compute
// non-exhaustive: cascading notes for capability errors

// Test that capability diagnostics for constructors print "TypeName.init"
// instead of the internal "$init" name.
// Regression test for https://github.com/shader-slang/slang/issues/10955

struct MyType
{
    int value;

    [require(hlsl)]
    __init(int v)
    {
        value = v;
    }
}

void caller()
{
    // Non-generic constructor: should show "MyType.init".
    let t = MyType(42);
}

[numthreads(1, 1, 1)]
void main()
/*CHECK:
     ^^^^ unavailable features in entry point
*/
{
    caller();
}
//CHECK: see using of 'MyType.init'
//CHECK: see definition of 'MyType.init'
