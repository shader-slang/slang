// Test that Optional<T> with a resource/opaque type T produces a diagnostic.
// Regression test for https://github.com/shader-slang/slang/issues/8277
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_NEG): -target spirv -DNEGATIVE
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_GEN): -target spirv -DGENERIC_MEMBER

#if defined(GENERIC_MEMBER)

// A generic member accessor's `none` escapes the front-end guard (T is abstract there), so the
// concrete opaque payload must be caught during IR specialization or SPIR-V emit ICEs on E99997.

struct StructWithOptionalTexture<T>
{
    Optional<T> maybeItem;
}

struct TextureHolder<T>
{
    T item;
    // CHECK_GEN-DAG: {{.*}}optional-resource-type.slang:[[# @LINE+1]]
    __subscript()->Optional<T> { get { return none; } }
    // CHECK_GEN-DAG: {{.*}}optional-resource-type.slang:[[# @LINE+1]]
    Optional<T> getNone() { return none; }
    // Nested `Optional<Optional<T>>`: the opaque check must recurse through Optional layers.
    // CHECK_GEN-DAG: {{.*}}optional-resource-type.slang:[[# @LINE+1]]
    Optional<Optional<T>> getNestedNone() { return none; }
    // Opaque check must recurse through both a struct and its inner `Optional` to the opaque leaf.
    // CHECK_GEN-DAG: {{.*}}optional-resource-type.slang:[[# @LINE+1]]
    Optional<StructWithOptionalTexture<T>> getStructNone() { return none; }
}

[vk::binding(0, 1)] TextureHolder<Texture2D> gHolderSubscript;
[vk::binding(1, 1)] TextureHolder<Texture2D> gHolderMethod;
[vk::binding(2, 1)] TextureHolder<Texture2D> gHolderNested;
[vk::binding(3, 1)] TextureHolder<Texture2D> gHolderStruct;

[shader("fragment")]
float4 fs_main() : SV_Target
{
    // Use each accessor's payload so every `MakeOptionalNone` survives to the post-specialization check.
    // CHECK_GEN-DAG: error[E41037]
    // CHECK_GEN-DAG: error[E41037]
    // CHECK_GEN-DAG: error[E41037]
    // CHECK_GEN-DAG: error[E41037]
    float4 a = gHolderSubscript[].value.Load(int3(0));
    float4 b = gHolderMethod.getNone().value.Load(int3(0));
    float4 c = gHolderNested.getNestedNone().value.value.Load(int3(0));
    float4 d = gHolderStruct.getStructNone().value.maybeItem.value.Load(int3(0));
    return a + b + c + d;
}

#elif !defined(NEGATIVE)

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

// --- generic specialization: valid at declaration, error fires at the opaque call site ---

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

// Negative: non-resource payloads must produce no diagnostic from either layer.
// CHECK_NEG-NOT: error[E30902]
// CHECK_NEG-NOT: error[E41037]

float withFloat(Optional<float> x) { return x.hasValue ? x.value : 0.0; }
float withInt(Optional<int> x) { return x.hasValue ? float(x.value) : 0.0; }

struct MyStruct { float v; };
float withStruct(Optional<MyStruct> x) { return x.hasValue ? x.value.v : 0.0; }

// A generic member accessor with a non-opaque payload must specialize and lower without E41037.
struct NonOpaqueHolder<T>
{
    T item;
    Optional<T> getNone() { return none; }
}

[shader("compute")]
[numthreads(1,1,1)]
void main(RWStructuredBuffer<float> buffer)
{
    NonOpaqueHolder<float> h;
    let o = h.getNone();
    buffer[0] = withFloat(none) + withInt(none) + withStruct(none) +
                (o.hasValue ? o.value : 0.0);
}

#endif
