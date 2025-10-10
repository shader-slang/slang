//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK: error 15302: cyclic `#include` of file

#include "b.slang"