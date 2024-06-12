//TEST:SIMPLE(filecheck=CHECK): -target spirv

// `TT` is not valid for defining a varying entrypoint parameter,
// and we should diagnose an error.

// CHECK: error 39028

struct TT
{
    Texture2D tex;    
}

[numthreads(1, 1, 1)]
void f(TT t)
{
    return;
}