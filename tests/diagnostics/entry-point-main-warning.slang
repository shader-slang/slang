// Test to check if we emit a warning and rename main to main_ on CUDA, CPU, metal backends

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -entry main -target cuda
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -entry main -target metal
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -entry main -target cpp
[shader("compute")]
[numthreads(1,1,1)]
void main(RWBuffer<float> output)
{
    output[0] = 1.0f;
}

// CHECK: warning 40100: entry point 'main' is not allowed, and has been renamed to 'main_0'
// CHECK: void main_