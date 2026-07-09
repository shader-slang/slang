// Test that Optional<T> with a resource/opaque type T produces a diagnostic.
// Regression test for https://github.com/shader-slang/slang/issues/8277
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_NEG): -target spirv -DNEGATIVE

#ifndef NEGATIVE

// --- parameter positions ---

// CHECK: error[E30902]
float doOperation(Optional<SamplerState> transformationInfo)
{
    if (transformationInfo.hasValue)
    {
        return 5.0;
    }
    return 0.0;
}

// CHECK: error[E30902]
float withTexture(Optional<Texture2D> t) { return 0.0; }

// CHECK: error[E30902]
float withBuffer(Optional<RWStructuredBuffer<float>> b) { return 0.0; }

// CHECK: error[E30902]
float withCbuffer(Optional<ConstantBuffer<float>> cb) { return 0.0; }

// --- variable declaration positions ---

// CHECK: error[E30902]
Optional<SamplerState> gSampler;

// CHECK: error[E30902]
Optional<Texture2D> gTexture;

// --- return type positions ---

// CHECK: error[E30902]
Optional<SamplerState> getSampler() { return none; }

// --- struct transitively containing an opaque field ---

struct WithResource
{
    float x;
    Texture2D tex;
}

// CHECK: error[E30902]
float withStructResource(Optional<WithResource> s) { return 0.0; }

// CHECK: error[E30902]
Optional<WithResource> gStructResource;

// --- generic specialization ---
// process<T> is valid at declaration; the error fires at the call site when
// T is specialized to an opaque type.

void process<T>(Optional<T> value) {}

[shader("compute")]
[numthreads(1,1,1)]
void main(RWStructuredBuffer<float> buffer)
{
    buffer[0] = doOperation(none);

    // CHECK: error[E30902]
    process<SamplerState>(none);
}

#else

// Negative: Optional<T> with non-resource types must not produce E30902.
// CHECK_NEG-NOT: error[E30902]

float withFloat(Optional<float> x) { return x.hasValue ? x.value : 0.0; }
float withInt(Optional<int> x) { return x.hasValue ? float(x.value) : 0.0; }

struct MyStruct { float v; };
float withStruct(Optional<MyStruct> x) { return x.hasValue ? x.value.v : 0.0; }

[shader("compute")]
[numthreads(1,1,1)]
void main(RWStructuredBuffer<float> buffer)
{
    buffer[0] = withFloat(none) + withInt(none) + withStruct(none);
}

#endif
