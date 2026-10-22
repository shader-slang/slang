//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -g2 -O0 -validate-ir

// Regression test for https://github.com/shader-slang/slang/issues/13841
// `accumulate` is specialized for the set of types that `f` can hold. The specialization clones
// the function's parameters before its body, so it repoints the `DebugVarStorage` decoration of
// `inout acc` at the debug variable cloned with the body; without that, the decoration would
// name a debug variable of the generic function and `-validate-ir` would reject it.

interface IFoo
{
    float get();
}

struct A : IFoo
{
    float x;
    float get() { return x; }
}

struct B : IFoo
{
    float y;
    int z;
    float get() { return y + float(z); }
}

void accumulate<T : IFoo>(T obj, inout float acc)
{
    for (int k = 0; k < 3; k++)
        acc += obj.get();
}

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    IFoo f;
    if (tid.x == 0)
        f = A(1.0);
    else
        f = B(2.0, 3);
    float acc = outBuf[1];
    accumulate(f, acc);
    outBuf[0] = acc;
}

// Both `acc` debug variables, the caller's local and the specialized callee's parameter, are
// declared on storage; a dropped link would give the parameter a `_dbgvar_acc` copy.
// CHECK: OpEntryPoint
// CHECK-LABEL: %main = OpFunction
// CHECK: DebugDeclare {{%[A-Za-z_0-9]+}} %acc {{%[0-9]+}}{{$}}
// CHECK-LABEL: %accumulate = OpFunction
// CHECK: [[ACC_PARAM:%[A-Za-z_0-9]+]] = OpFunctionParameter %_ptr_Function_float
// CHECK-NOT: %_dbgvar_acc
// CHECK: DebugDeclare {{%[A-Za-z_0-9]+}} [[ACC_PARAM]] {{%[0-9]+}}{{$}}
