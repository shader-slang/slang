//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang glsl tests/glsl/writeonly-read-master-parity.slang -stage compute -entry main -target spirv -emit-spirv-directly

// The GLSL `writeonly` memory qualifier reports E30119 only for a variable initializer and for
// the right-hand side of an assignment. Other uses, including reads through members, elements,
// swizzles, operators, calls and control flow, are not reported. `__ref_writeonly` parameters
// have their own, stricter rule and do not change this one.

#version 450
layout(local_size_x = 1) in;
layout(std430, binding = 0) writeonly buffer B { int d[]; } b;
layout(std430, binding = 1) buffer C { int n; int m[4]; } c;
layout(std430, binding = 2) buffer D { writeonly int w; writeonly ivec4 v; writeonly int a[4]; } dd;
layout(rgba8, binding = 3) writeonly uniform image2D img;
void takeIn(int v) {}
void takeInOut(inout int v) {}
void takeOut(out int v) { v = 0; }
void store(writeonly image2D im) { imageStore(im, ivec2(0, 0), vec4(1)); }
int retIt() { return dd.w; }
void main()
{
    int r1 = b.d[0];
    int r2 = b.d[0] + 1;
    c.n = b.d[0];
    c.n = b.d.length();
    b.d[0] = 1;
    b.d[1] += 1;
    int r3 = dd.w;
//CHECK:        ^ E30119
//CHECK:        ^ cannot read from writeonly, check modifiers.
    c.n = dd.w;
//CHECK:     ^ E30119
//CHECK:     ^ cannot read from writeonly, check modifiers.
    int r4 = dd.w + 1;
    takeIn(dd.w);
    takeInOut(dd.w);
    takeOut(dd.w);
    if (dd.w > 0) c.n = 2;
    int r5 = dd.v.x;
    int r6 = dd.a[1];
    c.m[0] = dd.a[2];
    dd.w++;
    dd.v.x = 3;
    imageStore(img, ivec2(0, 0), vec4(1));
    ivec2 sz = imageSize(img);
    c.n = sz.x;
    float f = dd.w;
//CHECK:         ^ E30119
//CHECK:         ^ cannot read from writeonly, check modifiers.
    switch (dd.w) { default: break; }
    int r7 = -dd.w;
    int r8 = ~dd.w;
    ivec2 r9 = dd.v.xy;
    store(img);
    c.n = dd.v.x;
}
