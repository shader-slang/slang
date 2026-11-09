//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

// Its source-level uint field describes GPU binding; HGBC stores an owned opaque registry.
// Raw packing must classify the target representation rather than walk that uint field.
import slang.pipeline_rt;

[host]
void main()
{
    let tracker = makeRaytracingPipeline(1);
    print(reinterpret<uint>(tracker));
}

// CHECK: error[E41014]: type cannot be packed for dynamic dispatch
