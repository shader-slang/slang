//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0

// Regression test for shader-slang/slang#11565 (non-copyable variant).
//
// A non-copyable result type lowers its initializer to return through a
// caller-provided out-parameter rather than by value, so the object under
// construction is a parameter rather than a fresh local. It must still appear
// as `this` in debug info, consistently with the by-value case and with a
// `[mutating]` method. The struct has no method, so the only `this` debug
// record in the module belongs to the initializer's out-parameter.
//
// Robust against -g2 embedding the source as a DebugSource OpString: the name
// string is captured from the genuine OpString record and the records are
// matched via captured SSA ids (see debug-info-user-init-this.slang).

//CHECK: [[THIS_STR:%[0-9]+]] = OpString "this"
//CHECK: [[THIS_VAR:%[A-Za-z0-9_]+]] = OpExtInst {{.*}} DebugLocalVariable [[THIS_STR]]
// Exactly one `this` debug variable: no second DebugLocalVariable names it.
//CHECK-NOT: OpExtInst {{.*}} DebugLocalVariable [[THIS_STR]]
//CHECK: OpExtInst {{.*}} DebugDeclare [[THIS_VAR]]

[__NonCopyableType]
struct Movable
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
    Movable m = Movable(1, 2);
    result[0] = m.val;
}
