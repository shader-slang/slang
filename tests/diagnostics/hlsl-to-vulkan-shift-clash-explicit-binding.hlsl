//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target glsl -profile ps_4_0 -entry main -fvk-t-shift 0 all -no-codegen

// Regression test: `UsedRanges::findRangeContaining(UInt)` used to return the
// slot number instead of the index of the matching range, so the Vulkan
// binding clash check below read `ranges[5]` of a one-element list.

struct Data
{
    float a;
};

// Explicit Vulkan binding 5; this is the only used range, at range index 0.
[[vk::binding(5, 0)]] ConstantBuffer<Data> e;

// With `-fvk-t-shift 0`, t5 also maps to Vulkan binding 5 and clashes with `e`.
Texture2D x : register(t5);
/*CHECK:
          ^ conflicting Vulkan inferred binding
          ^ conflicting vulkan inferred binding for parameter 'e'
*/

float4 main() : SV_TARGET
{
    return float4(1, 1, 1, 0);
}
