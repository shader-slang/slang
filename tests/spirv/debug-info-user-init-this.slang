//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0

// Regression test for shader-slang/slang#11565.
//
// A user-written initializer must expose the object under construction as a
// `this` debug variable, just as a [mutating] method exposes its implicit
// `this` parameter. Previously the initializer's result var carried no name
// hint or debug location, so the debug-var pass emitted no DebugLocalVariable
// for it and a debugger could not inspect the object inside the initializer.
//
// Robust against -g2 embedding the source (including these comment lines) as a
// DebugSource OpString: the name string is captured from the genuine OpString
// record (the embedded copy escapes its quotes and so cannot match), and the
// DebugLocalVariable / DebugDeclare are matched via captured SSA ids rather
// than the bare identifier. The struct has no method, so the only `this` debug
// record in the module belongs to the initializer.

//CHECK: [[THIS_STR:%[0-9]+]] = OpString "this"
//CHECK: [[THIS_VAR:%[A-Za-z0-9_]+]] = OpExtInst {{.*}} DebugLocalVariable [[THIS_STR]]
// Exactly one `this` debug variable: no second DebugLocalVariable names it.
//CHECK-NOT: OpExtInst {{.*}} DebugLocalVariable [[THIS_STR]]
//CHECK: OpExtInst {{.*}} DebugDeclare [[THIS_VAR]]

struct Simple
{
    int val;
    __init(int a, int b)
    {
        int sum = a + b;
        val = sum;
    }
};

RWStructuredBuffer<int> result;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Simple simple = Simple(1, 2);
    result[0] = simple.val;
}
