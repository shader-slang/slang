//TEST:SIMPLE(filecheck=CHECK): -target spirv -g2 -O0
// Tests generation of DebugLocalVariable and DebugDeclare for enum variables

//CHECK: [[E_STR:%[0-9]+]] = OpString "e"
//CHECK: [[E_VAR:%[A-Za-z0-9_]+]] = OpExtInst {{.*}} DebugLocalVariable [[E_STR]]
//CHECK: OpExtInst {{.*}} DebugDeclare [[E_VAR]]

enum MyEnum {
    VALUE_A,
    VALUE_B,
    VALUE_C
};

RWStructuredBuffer<MyEnum> out;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    MyEnum e = MyEnum.VALUE_A;
    out[0] = e;
}
