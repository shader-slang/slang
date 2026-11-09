//DIAGNOSTIC_TEST:SIMPLE(filecheck=HOST): -target hgbc -entry main -DHOST_GETTER=1
//DIAGNOSTIC_TEST:SIMPLE(filecheck=HOST): -target hgbc -entry main -DHOST_GETTER=2
//DIAGNOSTIC_TEST:SIMPLE(filecheck=DEVICE): -target spirv -entry generate -profile sm_6_6 -DDEVICE_CALL=1
//DIAGNOSTIC_TEST:SIMPLE(filecheck=DEVICE): -target spirv -entry generate -profile sm_6_6 -DDEVICE_CALL=2
//DIAGNOSTIC_TEST:SIMPLE(filecheck=DEVICE): -target spirv -entry generate -profile sm_6_6 -DDEVICE_CALL=3

// A tracker has an opaque host identity and a device offset. Its operations belong to one side.
import slang.pipeline_rt;

[raypayload]
struct Payload
{
    uint value : read(caller, miss)
        : write(caller, miss);
}

[shader("miss")]
void miss(inout Payload payload) {}

#if defined(HOST_GETTER)
[host]
uint main(RaytracingPipeline pipeline)
{
#if HOST_GETTER == 1
    return getBindingOffset(pipeline);
#else
    return pipeline.getBindingOffset();
#endif
}
#else
[shader("raygeneration")]
void emptyGenerate() {}

[shader("raygeneration")]
void generate(uniform RaytracingPipeline pipeline)
{
#if DEVICE_CALL == 1
    makeRaytracingPipeline(1);
#elif DEVICE_CALL == 2
    __setRayMiss(pipeline, 0, miss)();
#else
    __dispatchRays(emptyGenerate, uint3(1), 1)();
#endif
}
#endif

// HOST: entrypoint 'main' uses features that are not available in 'dispatch' stage for 'hgbc' compilation target
// HOST: note: see using of 'getBindingOffset'
// DEVICE: entrypoint 'generate' uses features that are not available in '_raygen' stage for 'spirv' compilation target
