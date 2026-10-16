//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Check that a read guarded by a separate uniform-buffer load is diagnosed.
// The pass does not equate conditions merely because their source expressions match.
struct Flags { bool flag; };
ConstantBuffer<Flags> ubo;
struct Data { float value; };

[shader("compute")]
void main(uniform float* output)
{
    Data data;
    if (ubo.flag) data.value = 1;
    if (ubo.flag) output[0] = data.value;
//CHECK:                    ^ E41035
//CHECK:                    ^ variable 'data' may be uninitialized on some paths
}
