// Test that a 64-bit specialization constant in numthreads is reported as unsupported on WGSL.
// A WGSL `@workgroup_size` argument must be an i32 or u32 override, so only `int`, `uint` and
// enums with those underlying types can name an override there.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):-target wgsl -entry main

[[vk::constant_id(0)]] const int64_t X = 4;
[numthreads(X, 1, 1)]
/*CHECK:
 ^^^^^^^^^^ Specialization constants are not supported in the 'numthreads' attribute
*/
[shader("compute")]
void main() {}
