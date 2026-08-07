//TEST:SIMPLE(filecheck=CHECK): -target metal -whole-program
//TEST:SIMPLE(filecheck=DEDUP): -target metal -whole-program

// An `export`/library-boundary function has no caller to specialize its
// parameter address spaces from, so an `out`/`inout` (mutable-reference
// pointer) parameter must be defaulted to `thread` when the exported function
// is seeded. Left as `AddressSpace::Generic` it reaches the Metal emitter's
// pointer address-space switch, which has no case for it.

// CHECK-DAG: void onb_{{[A-Za-z0-9_]*}}({{.*}}, {{.*}}thread{{.*}}* {{.*}}, {{.*}}thread{{.*}}* {{.*}})
export void onb(float3 n, out float3 b1, out float3 b2)
{
    const float s = n.z >= 0.0 ? 1.0 : -1.0;
    b1 = float3(s, 0, 0);
    b2 = float3(0, s, 0);
}

// CHECK-DAG: void advance_{{[A-Za-z0-9_]*}}({{.*}}thread{{.*}}* {{.*}})
export void advance(inout uint state)
{
    state = state * 747796405u + 2891336453u;
}

// An exported function calling another exported out-param function with local
// `thread` variables must emit the call against a matching `thread*` callee,
// and `onb` must be defined exactly once (see the DEDUP prefix below) so the
// seeded signature is reused rather than duplicated. Emitted MSL cannot say
// which IR node survived — specialization clones copy the export decoration
// and name — so the invariant here is call + single definition, not identity.
// CHECK-DAG: float3 buildFrame_{{[A-Za-z0-9_]*}}(
// CHECK: onb_{{[A-Za-z0-9_]*}}({{.*}}&{{.*}}, {{.*}}&{{.*}})
export float3 buildFrame(float3 n)
{
    float3 t, b;
    onb(n, t, b);
    return t + b;
}

// An exported root with no pointer parameter of its own, calling a
// *non-exported* `out`/`inout` helper. Defaulting only an exported function's
// own parameters is not enough: `scramble` has nothing to default, so its body
// must still be traversed or `mix2`'s `inout` pointer stays `Generic` and hits
// the same emitter switch one call level down. Pinning `mix2`'s two `thread*`
// parameters and the call to it is what distinguishes a real specialization
// from merely surviving compilation.
// CHECK-DAG: uint scramble_{{[A-Za-z0-9_]*}}(
// CHECK-DAG: void mix2_{{[A-Za-z0-9_]*}}(uint thread* {{.*}}, uint thread* {{.*}})
// CHECK: mix2_{{[A-Za-z0-9_]*}}({{.*}}&{{.*}}, {{.*}}&{{.*}})
void mix2(inout uint a, inout uint b)
{
    a ^= b << 3u;
    b ^= a >> 5u;
}
export uint scramble(uint seed)
{
    uint a = seed, b = seed ^ 0x9e3779b9u;
    mix2(a, b);
    return a + b;
}

// The `onb` call inside `buildFrame` requests exactly the `thread` address
// spaces the seeded exported `onb` already carries, so call-site specialization
// reuses that one canonical function instead of cloning a duplicate `thread*`
// variant. `onb` must therefore be defined exactly once. The DEDUP prefix runs
// as a separate FileCheck pass so this count is not perturbed by the CHECK-DAG
// matches above.
// DEDUP-COUNT-1: void onb_{{[A-Za-z0-9_]*}}(
// DEDUP-NOT: void onb_{{[A-Za-z0-9_]*}}(
