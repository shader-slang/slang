// shaders.slang

//
// This example builds on the simplistic shaders presented in the
// "Hello, World" example by adding support for (intentionally
// simplistic) surface materil and light shading.
//
// The code here is not meant to exemplify state-of-the-art material
// and lighting techniques, but rather to show how a shader
// library can be developed in a modular fashion without reliance
// on the C preprocessor manual parameter-binding decorations.
//

// We will start with a `struct` for per-view parameters that
// will be allocated into a `ParameterBlock`.
//
// As written, this isn't very different from using an HLSL
// `cbuffer` declaration, but importantly this code will
// continue to work if we add one or more resources (e.g.,
// an enironment map texture) to the `PerView` type.
//
struct PerView
{
    float4x4    viewProjection;

    float3      lightDir;
    float3      lightColor;
};
ParameterBlock<PerView>     gViewParams;

// Declaring a block for per-model parameter data is
// similarly simple.
//
struct PerModel
{
    float4x4    modelTransform;
    float4x4    inverseTransposeModelTransform;
};
ParameterBlock<PerModel>    gModelParams;


// Next, we are going to demonstrate a simplistic interface
// for surface materials. As written, materials can only
// determine how to compute the diffuse color component
// of a surface; a more advanced example would fold
// the entire BRDF into the material interface.
//
interface IMaterial
{
    float3 getDiffuseColor();
};

// In order for our shader to be able to take a material
// as a parameter, we need to declare a `ParameterBlock<M>`
// for some material type `M`. Rather than hard-code the
// specific material type to use, or select one via the
// preprocessor, we will use Slang's support for generics,
// by defining a "global type parameter":
//
type_param TMaterial : IMaterial;
//
// This declaration declares a shader parameter `TMaterial`
// that is a to-be-determined *type*. The `TMaterial`
// type parameter is *constrained* to only support types
// that implement our `IMaterial` interface.
//
// With the `TMaterial` parameter declared, we can
// declare that our shader takes as input a parameter block
// containing material data:
//
ParameterBlock<TMaterial>   gMaterial;

// For now, we will define only a single implementation
// of the `IMaterial` interface, which is a simple material
// with a uniform diffuse color:
//
struct SimpleMaterial : IMaterial
{
    float3 diffuseColor;

    float3 getDiffuseColor()
    {
        return diffuseColor;
    }
};
//
// Note that no other code in this file statically
// references the `SimpleMaterial` type, and instead
// it is up to the application to "plug in" this type,
// or another `IMaterial` implementation for the
// `TMaterial` parameter.
//

// Our vertex shader entry point is only marginally more
// complicated than the Hello World example. We will
// start by declaring the various "connector" `struct`s.
//
struct AssembledVertex
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : UV;
};
struct CoarseVertex
{
    float3 worldPosition;
    float3 worldNormal;
    float2 uv;
};
struct VertexStageOutput
{
    CoarseVertex    coarseVertex    : CoarseVertex;
    float4          sv_position     : SV_Position;
};

// Perhaps most interesting new feature of the entry
// point decalrations is that we use a `[shader(...)]`
// attribute (as introduced in HLSL Shader Model 6.x)
// in order to tag our entry points.
//
// This attribute informs the Slang compiler which
// functions are intended to be compiled as shader
// entry points (and what stage they target), so that
// the programmer no longer needs to specify the
// entry point name/stage through the API (or on
// the command line when using `slangc`).
//
// While HLSL added this feature only in newer versions,
// the Slang compiler supports this attribute across
// *all* targets, so that it is okay to use whether you
// want DXBC, DXIL, or SPIR-V output.
//
[shader("vertex")]
VertexStageOutput vertexMain(
    AssembledVertex assembledVertex)
{
    VertexStageOutput output;

    float3 position = assembledVertex.position;
    float3 normal   = assembledVertex.normal;
    float2 uv       = assembledVertex.uv;

    float3 worldPosition = mul(gModelParams.modelTransform, float4(position, 1.0)).xyz;
    float3 worldNormal = mul(gModelParams.inverseTransposeModelTransform, float4(normal, 0.0)).xyz;

    output.coarseVertex.worldPosition = worldPosition;
    output.coarseVertex.worldNormal   = worldNormal;
    output.coarseVertex.uv            = uv;

    output.sv_position = mul(gViewParams.viewProjection, float4(worldPosition, 1.0));

    return output;
}

// Our fragment shader is almost trivial, with the most interesting
// thing being how it uses the `TMaterial` type parameter (through the
// value stored in the `gMaterial` parameter block) to dispatch to
// the correct implementation of the `getDiffuseColor()` method
// in the `IMaterial` interface.
//
// The `gMaterial` parameter block declaration thus serves not only
// to group certain shader parameters for efficient CPU-to-GPU
// communication, but also to select the code that will execute
// in specialized versions of the `fragmentMain` entry point.
//
[shader("fragment")]
float4 fragmentMain(
    CoarseVertex coarseVertex : CoarseVertex) : SV_Target
{
    float3 N = normalize(coarseVertex.worldNormal);
    float3 L = normalize(gViewParams.lightDir);

    float4 color;
    color.xyz = gMaterial.getDiffuseColor() * max(0, dot(N, L));
    color.w = 1.0f;

    return color;
}
