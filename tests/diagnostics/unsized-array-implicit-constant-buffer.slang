// shader-slang/slang#13530: the compiler packs the ordinary data of global uniform
// parameters into an implicit constant buffer. Like an explicit `cbuffer`, that buffer
// cannot hold an unsized array of ordinary data, so we diagnose E31215 instead of
// producing an invalid layout. Resource-only unsized arrays stay valid (see
// unsized-array-implicit-constant-buffer-valid.slang).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main

uniform var input: float4[];
//CHECK:    ^^^^^ unsized type in constant buffer
//CHECK:    ^^^^^ cannot use unsized type 'vector<float,4>[]' in a constant buffer.
uniform var addend: float4;

uniform float4 nested[][2];
//CHECK:       ^^^^^^ unsized type in constant buffer
//CHECK:       ^^^^^^ cannot use unsized type 'vector<float,4>[2][]' in a constant buffer.

uniform unorm float4 normalized[];
//CHECK:             ^^^^^^^^^^ unsized type in constant buffer
//CHECK:             ^^^^^^^^^^ cannot use unsized type 'unorm vector<float,4>[]' in a constant buffer.

struct TrailingData
{
    float4 a;
    float4 x[];
//CHECK:   ^ unsized type in constant buffer
//CHECK:   ^ cannot use unsized type 'vector<float,4>[]' in a constant buffer.
}
uniform TrailingData trailing;
//CHECK:             ^^^^^^^^ see declaration of 'trailing'

// A resource field does not make the element resource-only: `v` is still ordinary data.
struct Mixed
{
    Texture2D t;
    float4 v;
}
uniform Mixed mixed[];
//CHECK:      ^^^^^ unsized type in constant buffer
//CHECK:      ^^^^^ cannot use unsized type 'Mixed[]' in a constant buffer.

// A pointer is an address stored in uniform memory, not a resource binding.
uniform float4* pointers[];
//CHECK:        ^^^^^^^^ unsized type in constant buffer
//CHECK:        ^^^^^^^^ cannot use unsized type 'Ptr<vector<float,4>

enum Mode
{
    A,
    B,
}
uniform Mode modes[];
//CHECK:     ^^^^^ unsized type in constant buffer
//CHECK:     ^^^^^ cannot use unsized type 'Mode[]' in a constant buffer.

uniform Tuple<Texture2D, float4> tuples[];
//CHECK:                         ^^^^^^ unsized type in constant buffer
//CHECK:                         ^^^^^^ cannot use unsized type 'Tuple<Texture2D<vector<float,4>>, vector<float,4>>[]' in a constant buffer.

// Each level of a nested generic struct is a distinct instantiation of the same declaration.
struct Box<T>
{
    T value;
}
uniform Box<Box<float4>> boxes[];
//CHECK:                 ^^^^^ unsized type in constant buffer
//CHECK:                 ^^^^^ cannot use unsized type 'Box<Box<vector<float,4>>>[]' in a constant buffer.

struct Pair<A, B>
{
    A first;
    B second;
}
uniform Pair<Texture2D, Pair<Texture2D, float4>> pairs[];
//CHECK:                                         ^^^^^ unsized type in constant buffer
//CHECK:                                         ^^^^^ cannot use unsized type 'Pair<Texture2D<vector<float,4>>, Pair<Texture2D<vector<float,4>>, vector<float,4>>>[]' in a constant buffer.

uniform Conditional<float4, true> present[];
//CHECK:                          ^^^^^^^ unsized type in constant buffer
//CHECK:                          ^^^^^^^ cannot use unsized type 'Conditional<vector<float,4>, 1>[]' in a constant buffer.

// The ordinary data of a base struct stays behind when legalization moves out `t`.
struct OrdinaryBase
{
    float4 a;
}
struct DerivedWithResource : OrdinaryBase
//CHECK:                     ^^^^^^^^^^^^ support for inheritance is unstable
{
    Texture2D t;
}
uniform DerivedWithResource derived[];
//CHECK:                    ^^^^^^^ unsized type in constant buffer
//CHECK:                    ^^^^^^^ cannot use unsized type 'DerivedWithResource[]' in a constant buffer.

SamplerState samplerState;
RWStructuredBuffer<float4> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    output[0] = input[2277] + addend + nested[3][1] + normalized[1] + trailing.x[3] + mixed[2].v +
                *pointers[1] + mixed[1].t.SampleLevel(samplerState, float2(0), 0) +
                (modes[1] == Mode.B ? 1 : 0) + tuples[1]._1 + boxes[1].value.value +
                pairs[2].second.second + present[1].get().value + derived[2].a;
}
