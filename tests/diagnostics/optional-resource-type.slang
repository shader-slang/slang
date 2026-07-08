// Test that Optional<T> with a resource/opaque type T produces a diagnostic.
// Regression test for https://github.com/shader-slang/slang/issues/8277
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_NEG): -target spirv -DNEGATIVE
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_GEN): -target spirv -DGENERIC_MEMBER

#if defined(GENERIC_MEMBER)

// --- generic member accessor returning `none` (regression for #7878) ---
// The `Optional<T>` return type is checked once with T abstract, so the
// front-end opaque-handle guard passes; the concrete `Optional<Texture2D>`
// only materializes during IR generic specialization. The diagnostic must
// fire there (E41037, the IR-layer variant of E30902), otherwise SPIR-V emit
// hits an unhandled `defaultConstruct` (E99997 internal error). This shape
// must reach IR lowering, so it lives in its own compilation unit with no
// front-end errors that would halt the compiler first. The `--> ...:line:col`
// checks pin each error to the exact `none` it is reported against.

struct TextureHolder<T>
{
    T item;
    // CHECK_GEN-DAG: {{.*}}optional-resource-type.slang:[[# @LINE+1]]
    __subscript()->Optional<T> { get { return none; } }
    // CHECK_GEN-DAG: {{.*}}optional-resource-type.slang:[[# @LINE+1]]
    Optional<T> getNone() { return none; }
    // A nested `Optional<Optional<T>>` payload is also opaque: `isOpaqueType`
    // does not recurse through `IROptionalType`, so the check must peel the
    // Optional layers before testing the leaf, or this `none` slips through to
    // the same `defaultConstruct` ICE.
    // CHECK_GEN-DAG: {{.*}}optional-resource-type.slang:[[# @LINE+1]]
    Optional<Optional<T>> getNestedNone() { return none; }
}

[vk::binding(0, 1)] TextureHolder<Texture2D> gHolderSubscript;
[vk::binding(1, 1)] TextureHolder<Texture2D> gHolderMethod;
[vk::binding(2, 1)] TextureHolder<Texture2D> gHolderNested;

[shader("fragment")]
float4 fs_main() : SV_Target
{
    // Force specialization of all three accessors of TextureHolder<Texture2D>
    // and use the payload, so the specialized `MakeOptionalNone` survives to
    // the post-specialization check (mirrors the original repro, which loaded
    // through the optional's payload). Each E41037 is pinned to its source
    // `none` by the `-->` locator checks above; the three header checks here
    // are order-independent (`-DAG`) and require three distinct occurrences,
    // asserting exactly the three expected errors regardless of emit order.
    // CHECK_GEN-DAG: error[E41037]
    // CHECK_GEN-DAG: error[E41037]
    // CHECK_GEN-DAG: error[E41037]
    float4 a = gHolderSubscript[].value.Load(int3(0));
    float4 b = gHolderMethod.getNone().value.Load(int3(0));
    float4 c = gHolderNested.getNestedNone().value.value.Load(int3(0));
    return a + b + c;
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

// Negative: Optional<T> with non-resource types must not produce a diagnostic,
// from either the front-end (E30902) or the post-specialization IR check (E41037).
// CHECK_NEG-NOT: error[E30902]
// CHECK_NEG-NOT: error[E41037]

float withFloat(Optional<float> x) { return x.hasValue ? x.value : 0.0; }
float withInt(Optional<int> x) { return x.hasValue ? float(x.value) : 0.0; }

struct MyStruct { float v; };
float withStruct(Optional<MyStruct> x) { return x.hasValue ? x.value.v : 0.0; }

// A generic member accessor returning `none` for a NON-opaque payload must reach
// IR specialization and lowering without firing E41037 (guards against the IR
// opaque test being broader than the language rule).
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
