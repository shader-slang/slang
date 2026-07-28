// A whole-program HLSL compile puts every entry point into one translation unit, so the
// `[shader("...")]` attribute is the only thing left that tells DXC which function is an
// entry point and what stage it has. Check that the attributes survive, and that the
// declarations shared by the two entry points are emitted once rather than once per entry.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -whole-program
//TEST:SIMPLE(filecheck=ONCE): -target hlsl -whole-program
//TEST:COMPILE: tests/hlsl/hlsl-whole-program-multi-entry.slang -target hlsl -o tests/hlsl/hlsl-whole-program-multi-entry.hlsl

// The shared helper and the shared payload struct each appear exactly once: emitted before
// the entry points, and not again afterwards.
// ONCE: uint sharedHelper{{[_0-9]*}}(uint
// ONCE: struct Payload
// ONCE-NOT: uint sharedHelper{{[_0-9]*}}(uint
// ONCE-NOT: struct Payload

// Both entry points keep their stage attribute.
// CHECK: [shader("closesthit")]
// CHECK: MainCH
// CHECK: [shader("anyhit")]
// CHECK: MainAH

struct Payload
{
    uint hit;
};

uint sharedHelper(uint a)
{
    return a + 3U;
}

[shader("closesthit")]
void MainCH(inout Payload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.hit = sharedHelper(1U);
}

[shader("anyhit")]
void MainAH(inout Payload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.hit = sharedHelper(2U);
}
