//TEST:SIMPLE(filecheck=CHECK): -target spirv -fvk-use-entrypoint-name
//TEST:SIMPLE(filecheck=CUDA): -target cuda

// CHECK: OpEntryPoint

// The ordinary-function copy of `innerMain` that `outerMain` calls must keep its `id` parameter;
// CUDA drops system-value parameters only from entry-point signatures.
// CUDA: innerMain_{{[0-9]+}}(int{{[^)]*}}id{{[^)]*}}){{$}}
// CUDA: innerMain_{{[0-9]+}}(&{{[^)]+}});

RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1,1,1)]
void innerMain(int id : SV_DispatchThreadID)
{
    output[id] = id;
}

[shader("compute")]
[numthreads(1,1,1)]
void outerMain(int id : SV_DispatchThreadID)
{
    innerMain(id);
}