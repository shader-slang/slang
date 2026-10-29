//TEST:SIMPLE(filecheck=CHECK): -target metal -entry vmain -stage vertex
//TEST:SIMPLE(filecheck=CHECK): -target metal -entry cmain -stage compute

// An exported function that no entry point calls is still emitted, but nothing specializes the
// address spaces of its reference parameters. On Metal, a by-value struct or array parameter is
// passed by const reference (see `transformParamsToConstRef`), and `out`/`inout` parameters are
// pointers, so each must get an address space even when the function has no call site. Address-
// space specialization gives them `thread`.

struct S
{
    float3 a;
    float b;
};

[shader("vertex")]
float4 vmain(float3 p: POSITION) : SV_Position
{
    return float4(p, 1);
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void cmain()
{
    outputBuffer[0] = 1;
}

// CHECK-DAG: float takeStruct_{{[0-9]+}}(const S_{{[0-9]+}} thread* {{[A-Za-z0-9_]+}})
export float takeStruct(S s)
{
    return s.a.x + s.b;
}

// CHECK-DAG: float takeArray_{{[0-9]+}}(const array<float, int(4)> thread* {{[A-Za-z0-9_]+}})
export float takeArray(float a[4])
{
    return a[1] + a[3];
}

// CHECK-DAG: void writeStruct_{{[0-9]+}}(S_{{[0-9]+}} thread* {{[A-Za-z0-9_]+}}, float thread* {{[A-Za-z0-9_]+}})
export void writeStruct(inout S s, out float r)
{
    s.b = s.b + 1;
    r = s.a.x;
}

// A user pointer is already `device`, so only the `out` parameter takes the `thread` default.
// CHECK-DAG: void readPtr_{{[0-9]+}}(int device* {{[A-Za-z0-9_]+}}, int thread* {{[A-Za-z0-9_]+}})
export void readPtr(int* p, out int r)
{
    r = *p;
}

// CHECK-DAG: float takeStructDll_{{[0-9]+}}(const S_{{[0-9]+}} thread* {{[A-Za-z0-9_]+}})
[DllExport]
float takeStructDll(S s)
{
    return s.a.y;
}
