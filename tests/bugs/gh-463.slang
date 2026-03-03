//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// undefined identifier used in `[instance(...)]` attribute

struct V { float4 u : U; };

[instance(COUNT)]
//CHECK:  ^^^^^ undefined identifier
//CHECK:  ^^^^^ undefined identifier 'COUNT'.
void myGS(triangle V input[3], TriangleStream<V> output)
{

}
