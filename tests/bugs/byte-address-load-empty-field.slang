// A void placeholder field (synthesized for an empty/zero-sized struct field) must not generate a
// byte-address buffer access: its zero stride otherwise produces a `Load<void>` (invalid on
// HLSL/DXIL) or a divide by zero on SPIR-V/GLSL. See shader-slang/slang#12711.
//
// The empty field is placed both first (`ItemFirst`) and in the middle (`ItemMid`) so a regression
// is caught regardless of position, and `ItemMid`'s trailing `float c` proves the following real
// field keeps its natural offset (base + 4, i.e. 20U) across the skipped field. The forbidden void
// access is checked by a dedicated `HLSLNOVOID` prefix whose only directive is a `CHECK-NOT`, so it
// scans the entire output (a `CHECK-NOT` interleaved with positive checks would only scan the region
// between them and could miss a first-position regression).

//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile sm_6_5 -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSLNOVOID): -target hlsl -profile sm_6_5 -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry main -stage compute

struct Empty {}

struct ItemFirst
{
    Empty a;
    float b;
}

struct ItemMid
{
    float a;
    Empty b;
    float c;
}

ByteAddressBuffer g_in;
RWByteAddressBuffer g_out;

[numthreads(1, 1, 1)]
void main()
{
    ItemFirst f = g_in.Load<ItemFirst>(0);
    g_out.Store<ItemFirst>(0, f);
    ItemMid m = g_in.Load<ItemMid>(16);
    g_out.Store<ItemMid>(16, m);
}

// The divide-by-zero over a void field's zero stride aborts SPIR-V/GLSL codegen before emission, so
// on those targets reaching entry-point emission at all is the guard against a regression.

// SPIRV: OpEntryPoint

// GLSL emits shader-storage-buffer element accesses (no `.Load<void>`); reaching `void main()`
// means legalization did not divide by the void field's zero stride.
// GLSL: void main

// The real `float` fields must still load and store; `ItemMid.c` at offset 20U proves the field
// after the skipped empty field keeps its natural offset.
// HLSL: Load<float
// HLSL: Load<float >(20U)
// HLSL: Store(

// No load of `void` may appear anywhere in the HLSL output (whole-output scan; see header note).
// A byte-address store emits `.Store(offset, value)` with no template argument, so there is no
// `Store<void>` form to forbid; the store path is covered by successful compilation plus the
// positive store check above.
// HLSLNOVOID-NOT: Load<void
