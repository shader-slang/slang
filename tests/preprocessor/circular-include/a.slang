//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Test that we can diagnose an error when there is a circular include.

// CHECK: error 15302: cyclic `#include` of file

#include "b.slang"