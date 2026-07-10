//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NEG): -target spirv -DNEGATIVE
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_GEN): -target spirv -DGENERIC_MEMBER

#if defined(GENERIC_MEMBER)

// Exercise recursive Optional and aggregate payload classification.

struct StructWithOptionalTexture<T>
{
    Optional<T> maybeItem;
}

struct TextureHolder<T>
{
    T item;
    __subscript()->Optional<T> { get { return none; } }
//CHECK_GEN:                                  ^^^^ E41037
//CHECK_GEN:                                  ^^^^ 'Texture2D' is a resource or opaque type
    Optional<T> getNone() { return none; }
//CHECK_GEN:                       ^^^^ E41037
//CHECK_GEN:                       ^^^^ 'Texture2D' is a resource or opaque type
    Optional<Optional<T>> getNestedNone() { return none; }
//CHECK_GEN:                                       ^^^^ E41037
//CHECK_GEN:                                       ^^^^ 'Optional<Texture2D>' is a resource or opaque type
    Optional<StructWithOptionalTexture<T>> getStructNone() { return none; }
//CHECK_GEN:                                                        ^^^^ E41037
//CHECK_GEN:                                                        ^^^^ 'StructWithOptionalTexture' is a resource or opaque type
}

[vk::binding(0, 1)] TextureHolder<Texture2D> gHolderSubscript;
[vk::binding(1, 1)] TextureHolder<Texture2D> gHolderMethod;
[vk::binding(2, 1)] TextureHolder<Texture2D> gHolderNested;
[vk::binding(3, 1)] TextureHolder<Texture2D> gHolderStruct;

[shader("fragment")]
float4 fs_main() : SV_Target
{
    // Keep all four none values live through specialization.
    float4 a = gHolderSubscript[].value.Load(int3(0));
    float4 b = gHolderMethod.getNone().value.Load(int3(0));
    float4 c = gHolderNested.getNestedNone().value.value.Load(int3(0));
    float4 d = gHolderStruct.getStructNone().value.maybeItem.value.Load(int3(0));
    return a + b + c + d;
}

#elif !defined(NEGATIVE)

float doOperation(Optional<SamplerState> transformationInfo)
//CHECK:          ^^^^^^^^ E30902
//CHECK:          ^^^^^^^^ 'SamplerState' is a resource or opaque type
{
    if (transformationInfo.hasValue)
    {
        return 5.0;
    }
    return 0.0;
}

float withTexture(Optional<Texture2D> t) { return 0.0; }
//CHECK:          ^^^^^^^^ E30902
//CHECK:          ^^^^^^^^ 'Texture2D<vector<float,4>>' is a resource or opaque type

float withBuffer(Optional<RWStructuredBuffer<float>> b) { return 0.0; }
//CHECK:         ^^^^^^^^ E30902
//CHECK:         ^^^^^^^^ 'RWStructuredBuffer<float, DefaultDataLayout>' is a resource or opaque type

float withCbuffer(Optional<ConstantBuffer<float>> cb) { return 0.0; }
//CHECK:          ^^^^^^^^ E30902
//CHECK:          ^^^^^^^^ 'ConstantBuffer<float, DefaultDataLayout>' is a resource or opaque type

Optional<SamplerState> gSampler;
/*CHECK:
^^^^^^^^ E30902
^^^^^^^^ 'SamplerState' is a resource or opaque type
*/

Optional<Texture2D> gTexture;
/*CHECK:
^^^^^^^^ E30902
^^^^^^^^ 'Texture2D<vector<float,4>>' is a resource or opaque type
*/

Optional<SamplerState> getSampler() { return none; }
//CHECK:                                     ^^^^ E30902
//CHECK:                                     ^^^^ 'SamplerState' is a resource or opaque type

struct WithResource
{
    float x;
    Texture2D tex;
}

float withStructResource(Optional<WithResource> s) { return 0.0; }
//CHECK:                 ^^^^^^^^ E30902
//CHECK:                 ^^^^^^^^ 'WithResource' is a resource or opaque type

Optional<WithResource> gStructResource;
/*CHECK:
^^^^^^^^ E30902
^^^^^^^^ 'WithResource' is a resource or opaque type
*/

void process<T>(Optional<T> value) {}

[shader("compute")]
[numthreads(1,1,1)]
void main(RWStructuredBuffer<float> buffer)
{
    buffer[0] = doOperation(none);

    process<SamplerState>(none);
//CHECK:                  ^^^^ E30902
//CHECK:                  ^^^^ 'SamplerState' is a resource or opaque type
}

#else

float withFloat(Optional<float> x) { return x.hasValue ? x.value : 0.0; }
float withInt(Optional<int> x) { return x.hasValue ? float(x.value) : 0.0; }

struct MyStruct { float v; };
float withStruct(Optional<MyStruct> x) { return x.hasValue ? x.value.v : 0.0; }

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
