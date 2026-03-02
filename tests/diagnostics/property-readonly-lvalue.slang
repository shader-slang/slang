//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
// use read-only property as l-value

struct MyType
{
    property float3 prop { get { return float3(1, 2, 3);}}
}

void test()
{
    MyType t;
    t.prop.x += 1.0;
//CHECK:   ^ argument must be l-value
//CHECK:   ^ argument passed to parameter '0' must be l-value.
}
