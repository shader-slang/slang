// A swizzle is writable only if its base is. Vector swizzles already followed this rule; matrix
// and tuple swizzles of an immutable value used to be accepted as assignment targets.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

float matrixLet()
{
    let m = float4x4(0);
    m._m00 = 1;
//CHECK:   ^ E30011
//CHECK:   ^ left of '=' is not an l-value.
    return m[0][0];
}

float matrixConstParam(const float4x4 m)
{
    m._m00_m11 = float2(1, 2);
//CHECK:       ^ E30011
//CHECK:       ^ left of '=' is not an l-value.
    return m[0][0];
}

float tupleLet()
{
    let t = makeTuple(1.0, 2.0);
    t._0 = 5;
//CHECK: ^ E30011
//CHECK: ^ left of '=' is not an l-value.
    return t._0;
}

float vectorLet()
{
    let v = float4(0);
    v.x = 5;
//CHECK:^ E30011
//CHECK:^ left of '=' is not an l-value.
    return v.x;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = matrixLet() + matrixConstParam(float4x4(0)) + tupleLet() + vectorLet();
}
