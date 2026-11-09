//TEST:INTERPRET(hgvm):
//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

void fail(String message)
{
    hgbc_asm
    {
        OpFail $message;
    };
}

[noinline]
float evaluateScalar(float left, float right)
{
    return dot(left, right);
}

[noinline]
float evaluateFloat(float3 left, float3 right)
{
    return dot(left, right);
}

[noinline]
half evaluateHalf(half4 left, half4 right)
{
    return dot(left, right);
}

[noinline]
int evaluateInt(int4 left, int4 right)
{
    return dot(left, right);
}

[noinline]
uint evaluateUInt(uint2 left, uint2 right)
{
    return dot(left, right);
}

[noinline]
float evaluateFloat1(vector<float, 1> left, vector<float, 1> right)
{
    return dot(left, right);
}

[host]
void main()
{
    if (evaluateScalar(2.0f, 3.0f) != 6.0f)
        fail("HGVM scalar dot produced the wrong value");
    if (evaluateFloat(float3(1.5f, -2.0f, 4.0f), float3(2.0f, 3.0f, 0.5f)) != -1.0f)
        fail("HGVM float-vector dot produced the wrong value");
    if (float(evaluateHalf(half4(1.0h, 2.0h, 3.0h, 4.0h), half4(4.0h, 3.0h, 2.0h, 1.0h))) !=
        20.0f)
    {
        fail("HGVM half-vector dot produced the wrong value");
    }
    if (evaluateInt(int4(-2, 3, 4, -5), int4(6, -7, 2, -1)) != -20)
        fail("HGVM signed-integer-vector dot produced the wrong value");
    if (evaluateUInt(uint2(7, 11), uint2(3, 5)) != 76)
        fail("HGVM unsigned-integer-vector dot produced the wrong value");
    if (evaluateFloat1(vector<float, 1>(2.5f), vector<float, 1>(4.0f)) != 10.0f)
        fail("HGVM one-lane-vector dot produced the wrong value");
}

// CHECK: OpExtractElement
// CHECK: OpMul
// CHECK: OpAdd
