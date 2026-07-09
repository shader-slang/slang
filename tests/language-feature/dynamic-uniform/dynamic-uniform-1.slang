

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -profile cs_6_0 -validate-uniformity

RWStructuredBuffer<int> output;
void expectUniform(dynamic_uniform int v)
{
    output[0] = v;
}

[NonUniformReturn]
int produceNonUniform() { return 0; }

struct B
{
    int a;
    int c;
    dynamic_uniform int d;
    dynamic_uniform int d1;
    dynamic_uniform int d2;
    dynamic_uniform int d3;
    dynamic_uniform int d4;
}

int getBa(B obj)
{
    return obj.a;
}

int getBaFromInout(inout B obj)
{
    return obj.a;
}

int getBdFromInout(inout B obj)
{
    return obj.d;
}

ConstantBuffer<B> ub;
void main(int tid: SV_DispatchThreadID)
{
    int d = ub.a + ub.c;
    int f = ub.a + tid;
    int ff = ub.a + produceNonUniform();
    
    // Warning expected on next line
    expectUniform(f);
    // Warning expected on next line
    expectUniform(ff);

    B obj = ub;
    // Warning expected on next line
    expectUniform(getBaFromInout(obj));
    obj.a = ff;
    // Warning expected on next line
    expectUniform(getBa(obj));

    // Warning expected on next line
    obj.d = ff;

    int t = 0;
    if (tid == 0)
        t = 1;

    // Warning expected on next line
    obj.d1 = t;

    int u = 0;
    if (ub.a == 0)
        u = 1;
    // No warning expected here
    obj.d2 = u;

    int vv = 0;
    if (tid == 0)
        vv = 2;
    else
        vv = 2;
    // No warning expected here
    obj.d3 = vv;

    // No warning expected here
    expectUniform(getBdFromInout(obj));

    // No warning expected here
    obj.d4 = d; // OK

    // No warning expected here
    expectUniform(d);

    // No warning expected here
    expectUniform(getBa(ub));

    // No warning expected here
    expectUniform(asDynamicUniform(ff));

// CHECK-COUNT-7: warning[E412
// CHECK-NOT: warning[E41201]
// CHECK-NOT: warning[E41203]

}
