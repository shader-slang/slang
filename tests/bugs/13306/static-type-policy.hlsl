//TEST:SIMPLE_EX(filecheck=RESOURCE): tests/bugs/13306/static-type-policy.hlsl -Gec -no-codegen -DTRY_RESOURCE
//TEST:SIMPLE_EX(filecheck=PARAMETER_BLOCK): tests/bugs/13306/static-type-policy.hlsl -Gec -no-codegen -DTRY_PARAMETER_BLOCK
//TEST:SIMPLE_EX(filecheck=UNSIZED): tests/bugs/13306/static-type-policy.hlsl -Gec -no-codegen -DTRY_UNSIZED

// Semantic checking applies the same type restrictions to ordinary mutable `static` globals
// and compatibility shadows. A shadow remains a read-only alias when mutable storage is not
// supported, whereas an explicitly mutable `static` declaration requires a diagnostic.
struct Box<T>
{
    T value;
};
struct ResourceBase
{
    Texture2D<float4> texture;
};
struct ResourceDerived : ResourceBase
{
    uint value;
};
struct Data
{
    uint value;
};

#ifdef TRY_RESOURCE
static Box<Box<Texture2D<float4>>> mutableResource;
// RESOURCE: error[E30076]
static ResourceDerived mutableInheritedResource;
// RESOURCE: error[E30076]
uniform Box<Box<Texture2D<float4>>> resourceParameter;

void writeResource()
{
    resourceParameter = resourceParameter;
    // RESOURCE: error[E30011]
}
#endif

#ifdef TRY_PARAMETER_BLOCK
static ParameterBlock<Data> mutableBlock;
// PARAMETER_BLOCK: error[E30088]
uniform ParameterBlock<Data> blockParameter;

void writeBlock()
{
    blockParameter = blockParameter;
    // PARAMETER_BLOCK: error[E30011]
}
#endif

#ifdef TRY_UNSIZED
static uint mutableArray[];
// UNSIZED: error[E30071]
uniform uint arrayParameter[];
// UNSIZED: error[E31215]

void writeArray()
{
    arrayParameter[0] = 1;
    // UNSIZED: error[E30011]
}
#endif
