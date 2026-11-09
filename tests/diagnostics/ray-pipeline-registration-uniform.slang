//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hgbc

import slang.pipeline_rt;

[raypayload]
struct Payload
{
    uint value : read(caller, miss) : write(caller, miss);
}

[shader("miss")]
void miss(inout Payload payload, uint value)
{
    payload.value = value;
}

[host]
void main()
{
    let pipeline = makeRaytracingPipeline(1);
    __setRayMiss(pipeline, 0, miss)(3);
//CHECK:                      ^^^^ invalid ray-tracing shader signature
//CHECK:                      ^^^^ miss shader extra shader parameters must be explicitly uniform
}
