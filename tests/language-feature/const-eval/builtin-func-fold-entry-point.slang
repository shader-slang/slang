// A folded built-in call can size a local array and set `[numthreads]` in a compute entry point
// (shader-slang/slang#13973).
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry main -stage compute

RWStructuredBuffer<int> buf;
static const int A = 8;
static const int B = 4;

// HLSL: [numthreads(8, 1, 1)]
// HLSL: buf_0[int(0)] = int(10);
// WGSL: @workgroup_size(8, 1, 1)
// WGSL: buf_0[i32(0)] = i32(10);
[shader("compute")]
[numthreads(max(A, B), 1, 1)]
void main()
{
    int arr[max(A, B) + 2];
    buf[0] = arr.getCount();
}
