//DIAGNOSTIC_TEST:SIMPLE:

float foo(out float3 v)
{
    float r = v.x + 1.0;
    return r;
}
