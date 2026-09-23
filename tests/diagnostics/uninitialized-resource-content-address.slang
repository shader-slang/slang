// An address into a resource's contents does not alias the variable that stores the resource value.
// Forming the address still reads that value, so a later store through the address cannot make an
// uninitialized resource value valid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    RWStructuredBuffer<int> buffer;
    buffer[0] = 1;
//CHECK:      ^ use of uninitialized variable
//CHECK:      ^ use of uninitialized variable 'buffer'
    output[0] = buffer[0];
//CHECK:      ^ use of uninitialized variable
//CHECK:      ^ use of uninitialized variable 'buffer'
}
