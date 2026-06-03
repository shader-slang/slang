// Regression for #11095: synthesized default ctor for a struct whose member
// type is declared *after* it and contains a type with an explicit ctor used
// to fail with "not enough arguments to call", because
// `constructDefaultConstructorForType` called `_getDefaultCtor` without
// first ensuring the member struct had reached `AttributesChecked`.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage compute

struct SpectralSample
{
    float values[4];
    __init(const float f = 0.0f)
    {
        for (uint i = 0; i < 4; ++i)
            values[i] = f;
    }
};

struct leaf_bsdf_data
{
    SpectralSample tint;
    float roughness;
};

// Forward-declared member type: `weighted_layer_bsdf_data` is defined after
// the struct that uses it.  Before the fix, the synthesized ctor here
// inherited a required parameter and the outer `= {}` failed.
struct fresnel_layer_bsdf_data
{
    leaf_bsdf_data layer;
    weighted_layer_bsdf_data base;
};

struct weighted_layer_bsdf_data
{
    SpectralSample weight;
    leaf_bsdf_data layer;
    leaf_bsdf_data base;
};

struct outer
{
    fresnel_layer_bsdf_data data;
};

void init(inout outer o)
{
    o = {};
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outer o;
    init(o);
    // CHECK: void main
}
