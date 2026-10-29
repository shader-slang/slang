//TEST:SIMPLE(filecheck=CHECK): -target cpp -entry main -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry main -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=OBF): -target cpp -entry main -stage compute -line-directive-mode none -obfuscate

// A field of an `__extern_cpp` struct keeps its source name in C++ and CUDA
// output (https://github.com/shader-slang/slang/issues/14028). The backing
// storage that the checker synthesizes for bitfields has no source name, so it
// keeps a generated name: its `$`-prefixed internal name is not valid C++.
// Under `-obfuscate` the fields keep their hashed names.

//CHECK: struct Packed
//CHECK-NEXT: {
//CHECK-NOT: $
//CHECK: bit_field_backing
//CHECK-NEXT: float c;
//CHECK-NEXT: };

//OBF: struct _Sh{{[0-9A-F]+}}
//OBF-NEXT: {
//OBF-NEXT: uint32_t _Sh{{[0-9A-F]+}};
//OBF-NEXT: float _Sh{{[0-9A-F]+}};
//OBF-NEXT: };

__extern_cpp struct Packed
{
    uint a : 3;
    uint b : 5;
    float c;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
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
