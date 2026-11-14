//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl

// A `static const` global may alias a resource or sampler shader parameter
// (https://github.com/shader-slang/slang/issues/14378), but only when the value itself
// is made only of resources and samplers. Non-resource values read from shader
// parameters are runtime values and must still be diagnosed.

struct Params
{
    Texture2D t;
    float k;
}
ParameterBlock<Params> pb;

struct S
{
    float x;
}
ConstantBuffer<S> cb;

struct Mixed
{
    Texture2D t;
    float k;
}
Mixed m;

static const float pbScalar = pb.k;
//CHECK:           ^^^^^^^^ static const global initializer must be a compile-time constant
//CHECK:           ^^^^^^^^ initializer of static const global 'pbScalar' does not evaluate to a compile-time constant

static const float cbScalar = cb.x;
//CHECK:           ^^^^^^^^ static const global initializer must be a compile-time constant
//CHECK:           ^^^^^^^^ initializer of static const global 'cbScalar' does not evaluate to a compile-time constant

static const Mixed mixed = m;
//CHECK:           ^^^^^ static const global initializer must be a compile-time constant
//CHECK:           ^^^^^ initializer of static const global 'mixed' does not evaluate to a compile-time constant

// Fields inherited from a base struct are not resources, so `Derived` is not made only of
// resources even though its own field is a texture.
struct Base
{
    float k;
}
struct Derived : Base
//CHECK:         ^^^^ warning
{
    Texture2D t;
}
Derived derived;

static const Derived derivedAlias = derived;
//CHECK:             ^^^^^^^^^^^^ static const global initializer must be a compile-time constant
//CHECK:             ^^^^^^^^^^^^ initializer of static const global 'derivedAlias' does not evaluate to a compile-time constant

// Reading data through a resource handle, such as a texel or a buffer element, is a runtime
// value. Only the handle itself is exempt, so these must still be diagnosed.
Texture2D tex;
StructuredBuffer<float4> sb;

static const float4 texel = tex[uint2(0, 0)];
//CHECK:            ^^^^^ static const global initializer must be a compile-time constant
//CHECK:            ^^^^^ initializer of static const global 'texel' does not evaluate to a compile-time constant

static const float4 element = sb[0];
//CHECK:            ^^^^^^^ static const global initializer must be a compile-time constant
//CHECK:            ^^^^^^^ initializer of static const global 'element' does not evaluate to a compile-time constant
