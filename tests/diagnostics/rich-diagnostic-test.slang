// Test file for rich diagnostic formatting
// This file is designed to trigger a type mismatch diagnostic

//TEST:COMPILE: -target spirv

void process(int x)
{
    // Simple function that expects an int
}

[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    // This should trigger a type mismatch diagnostic
    // when calling process with a float argument
    float value = 1.5f;
    process(value);  // Error: expected int, got float
}