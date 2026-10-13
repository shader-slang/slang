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
/*CHECK:
      ^^^^ property or subscript cannot be written
      ^^^^ cannot write this property because it has no `set` or `ref` accessor
*/
}
