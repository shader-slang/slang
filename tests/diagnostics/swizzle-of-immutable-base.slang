// A swizzle is writable only if its base is and it has no duplicate components. Vector swizzles
// already followed this rule; matrix and tuple swizzles of an immutable value used to be accepted
// as assignment targets and as `out`/`inout` arguments.

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

float matrixElevenLet()
{
    let m = float4x4(0);
    m._11 = 1;
//CHECK:  ^ E30011
//CHECK:  ^ left of '=' is not an l-value.
    return m[0][0];
}

float tupleMultiElementLet()
{
    let t = makeTuple(1.0, 2.0);
    t._0_1 = makeTuple(3.0, 4.0);
//CHECK:   ^ E30011
//CHECK:   ^ left of '=' is not an l-value.
    return t._0;
}

float4x4 getMatrix()
{
    return float4x4(0);
}

void matrixRValue()
{
    getMatrix()._m00 = 1;
//CHECK:             ^ E30011
//CHECK:             ^ left of '=' is not an l-value.
}

void writeOut(out float x)
{
    x = 1;
}

void addInOut(inout float x)
{
    x += 1;
}

float matrixLetOutArg()
{
    let m = float4x4(0);
    writeOut(m._m00);
//CHECK:       ^^^^ E30047
//CHECK:       ^^^^ argument passed to parameter '0' must be l-value.
    return m[0][0];
}

float tupleLetInOutArg()
{
    let t = makeTuple(1.0, 2.0);
    addInOut(t._0);
//CHECK:       ^^ E30047
//CHECK:       ^^ argument passed to parameter '0' must be l-value.
    return t._0;
}

float matrixLetCompoundAssign()
{
    let m = float4x4(0);
    m._m00 += 1;
/*CHECK:
      ^^^^ E30047
      ^^^^ argument passed to parameter '0' must be l-value.
*/
    m._11++;
/*CHECK:
      ^^^ E30047
      ^^^ argument passed to parameter '0' must be l-value.
*/
    return m[0][0];
}

float matrixVarDuplicates()
{
    var m = float4x4(0);
    m._m00_m00 = float2(1, 2);
//CHECK:       ^ E30011
//CHECK:       ^ left of '=' is not an l-value.
    return m[0][0];
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    matrixRValue();
    outputBuffer[0] = matrixLet() + matrixConstParam(float4x4(0)) + tupleLet() + vectorLet() +
                      matrixElevenLet() + tupleMultiElementLet() + matrixLetOutArg() +
                      tupleLetInOutArg() + matrixLetCompoundAssign() + matrixVarDuplicates();
}
