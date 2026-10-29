//TEST:SIMPLE(filecheck=CHECK): -target cpp -entry main -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry main -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=OBF): -target cpp -entry main -stage compute -line-directive-mode none -obfuscate

// A field of an `__extern_cpp` struct keeps its source name in C++ and CUDA
// output (https://github.com/shader-slang/slang/issues/14028). The backing
// storage that the checker synthesizes for bitfields has no source name, and its
// `$`-prefixed internal name is not valid C++, so it gets that name without the
// `$` plus a number that no other member uses.
// Under `-obfuscate` the fields keep their hashed names.

//CHECK: struct Packed
//CHECK-NEXT: {
//CHECK-NEXT: bit_field_backing_0_1;
//CHECK-NEXT: float c;
//CHECK-NEXT: float bit_field_backing_0_0;
//CHECK-NEXT: };

//OBF: struct _Sh{{[0-9A-F]+}}
//OBF-NEXT: {
//OBF-NEXT: uint32_t _Sh{{[0-9A-F]+}};
//OBF-NEXT: float _Sh{{[0-9A-F]+}};
//OBF-NEXT: float _Sh{{[0-9A-F]+}};
//OBF-NEXT: };

__extern_cpp struct Packed
{
    uint a : 3;
    uint b : 5;
    float c;
    float bit_field_backing_0_0;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    Packed p;
    p.a = 1;
    p.b = 2;
    p.c = 3.0;
    outputBuffer[0] = p.c + p.a + p.b;
}
