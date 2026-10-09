//TEST:SIMPLE(filecheck=CHECK): -Gec -no-codegen -DDEEP_FIRST
//TEST:SIMPLE(filecheck=CHECK): -Gec -no-codegen

// Semantic checking must recognize resource fields in either declaration order and reject
// writes to the parameter's private struct value. The same resource-containing type occurs
// behind a deep chain of fields and as a direct field of `Parameters`.
struct Box<T> { T value; };
struct ResourceValue { Texture2D<float4> texture; };

// These macros form a finite chain of 126 `Box` fields. Inspecting the deep occurrence of
// `ResourceValue` reaches the type-tag query's nesting limit before its texture field.
// A later direct occurrence must still establish that resource property, instead of reusing
// an incomplete result from the deeper inspection.
#define BOX2(T) Box<Box<T>>
#define BOX4(T) BOX2(BOX2(T))
#define BOX8(T) BOX4(BOX4(T))
#define BOX16(T) BOX8(BOX8(T))
#define BOX32(T) BOX16(BOX16(T))
#define BOX64(T) BOX32(BOX32(T))
typedef BOX64(BOX32(BOX16(BOX8(BOX4(BOX2(ResourceValue)))))) DeepResource;

struct Parameters
{
#ifdef DEEP_FIRST
    DeepResource deep;
    ResourceValue direct;
#else
    ResourceValue direct;
    DeepResource deep;
#endif
};
uniform Parameters input;
Texture2D<float4> other;

void writeResource()
{
    input.direct.texture = other;
}

// CHECK: error[E30011]
