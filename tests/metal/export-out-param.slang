//TEST:SIMPLE(filecheck=CHECK): -target metal -whole-program
//TEST:SIMPLE(filecheck=DEDUP): -target metal -whole-program

// An `export`/library-boundary function with an `out` or `inout` parameter has
// no caller to specialize its parameter address space from, so before this fix
// its mutable-reference pointer parameter reached the Metal emitter still
// carrying `AddressSpace::Generic` and hit the emitter's "Unknown addressspace
// encountered" internal error. The address-space specialization pass now seeds
// exported functions and defaults such parameters to the `thread` address
// space, matching the default used for local mutable storage.

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

// An exported function that itself calls another exported out-param function
// (passing local `thread` variables) must still emit.
// CHECK-DAG: float3 buildFrame_{{[A-Za-z0-9_]*}}(
export float3 buildFrame(float3 n)
{
    float3 t, b;
    onb(n, t, b);
    return t + b;
}

// An exported root whose own signature has no pointer parameter, but whose body
// calls a *non-exported* `out`/`inout` helper. This is the case that fails if
// the pass only defaults an exported function's own parameters: `scramble` has
// nothing to default, so unless its body is still traversed the internal call to
// the non-exported `mix2` is never specialized and `mix2`'s `inout` pointer
// reaches the emitter carrying `AddressSpace::Generic` — the same crash one call
// level down. `mix2` is not exported, so it is emitted only as the specialized
// `thread*` callee reached through `scramble`.
// CHECK-DAG: uint scramble_{{[A-Za-z0-9_]*}}(
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
