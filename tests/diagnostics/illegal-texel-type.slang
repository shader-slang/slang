//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

Texture2D<float4x4> t1;
/*CHECK:
^^^^^^^^^ type argument doesn't conform to interface
^^^^^^^^^ type argument 'matrix<float,4,4>' does not conform to the required interface 'ITexelElement'
*/

RWBuffer<Sampler1D> t2;
/*CHECK:
^^^^^^^^ type argument doesn't conform to interface
^^^^^^^^ type argument 'Sampler1D<vector<float,4>>' does not conform to the required interface 'ITexelElement'
*/