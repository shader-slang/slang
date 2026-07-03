//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none
//TEST:REFLECTION(filecheck=REFLECT): -stage compute -entry computeMain -target cuda -no-codegen

// A CUDA compute entry-point `uniform` parameter that carries a descriptor table —
// here a struct nesting a fixed-size array of pointer-backed tensor structs, the
// shape SlangPy's functional API generates — is passed by reference: parameter
// binding lays it out as an implicit ParameterBlock (one 8-byte device pointer in
// kernel-argument space, payload in global memory), and the IR is reconciled to
// that recorded layout. A runtime index into the array is then an ordinary
// dynamically-addressed global load instead of a serial `.param`-space load chain
// (issue #11774).

struct MyTensor
{
    float* data;
    uint stride;
}

struct TensorList
{
    MyTensor tensors[32];
}

// The kernel receives the parameter as a pointer, and the runtime index goes
// through it; the remaining parameters keep the by-value kernel-argument path.
// CHECK: __global__ void computeMain(TensorList_0* list_0, StructuredBuffer<uint> indices_0, RWStructuredBuffer<float> output_0)
// CHECK: list_0->tensors_0[

// Reflection reports the truth about the ABI: the parameter is a parameter block
// occupying 8 bytes of kernel-argument space, its element preserving the full
// array layout (count and stride), and the following parameter starts after the
// pointer.
// REFLECT: "name": "list"
// REFLECT: "binding": {"kind": "uniform", "offset": 0, "size": 8
// REFLECT: "kind": "parameterBlock"
// REFLECT: "kind": "array"
// REFLECT: "elementCount": 32
// REFLECT: "name": "indices"
// REFLECT: "binding": {"kind": "uniform", "offset": 8

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform TensorList list,
    uniform StructuredBuffer<uint> indices,
    uniform RWStructuredBuffer<float> output)
{
    output[tid.x] = *(list.tensors[indices[tid.x]].data + tid.x);
}
