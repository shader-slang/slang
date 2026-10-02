//TEST:SIMPLE(filecheck=CHECK2): -target wgsl -entry copy2 -stage compute
//TEST:SIMPLE(filecheck=CHECK3): -target wgsl -entry copy3 -stage compute
//TEST:SIMPLE(filecheck=MEMBER): -target wgsl -entry copyMember -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry copy2 -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry copy3 -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry copyMember -stage compute

// A scalar array in a WGSL constant buffer is stored as an array of `vec4`, so
// each element is read back through a vector-to-scalar `vectorReshape`. Every
// temporary holding such an element must be declared before it is used, and no
// element read may be left behind as a discarded expression statement.

struct S2 { float a[2]; };
struct S3 { float a[3]; };

cbuffer C2 { float a2[2]; }
cbuffer C3 { float a3[3]; }
cbuffer CS { S2 s; }

RWStructuredBuffer<S2> out2;
RWStructuredBuffer<S3> out3;
RWStructuredBuffer<S2> outMember;

// CHECK2-LABEL: fn copy2
// CHECK2-NOT: {{^ *}}C2_0.{{.*}};
// CHECK2: var [[T1:_S[0-9]+]] : f32 = C2_0.a2_0.data_0[i32(1)].x;
// CHECK2-NOT: {{^ *}}C2_0.{{.*}};
// CHECK2: out2_0[i32(0)].a_0[i32(1)] = [[T1]];
[numthreads(1, 1, 1)]
void copy2() { out2[0].a = a2; }

// CHECK3-LABEL: fn copy3
// CHECK3-NOT: {{^ *}}C3_0.{{.*}};
// CHECK3: var [[T1:_S[0-9]+]] : f32 = C3_0.a3_0.data_0[i32(1)].x;
// CHECK3-NOT: {{^ *}}C3_0.{{.*}};
// CHECK3: var [[T2:_S[0-9]+]] : f32 = C3_0.a3_0.data_0[i32(2)].x;
// CHECK3-NOT: {{^ *}}C3_0.{{.*}};
// CHECK3: out3_0[i32(0)].a_0[i32(1)] = [[T1]];
// CHECK3: out3_0[i32(0)].a_0[i32(2)] = [[T2]];
[numthreads(1, 1, 1)]
void copy3() { out3[0].a = a3; }

// MEMBER-LABEL: fn copyMember
// MEMBER-NOT: {{^ *}}CS_0.{{.*}};
// MEMBER: var [[T1:_S[0-9]+]] : f32 = CS_0.s_0.a_0.data_0[i32(1)].x;
// MEMBER-NOT: {{^ *}}CS_0.{{.*}};
// MEMBER: outMember_0[i32(0)].a_0[i32(1)] = [[T1]];
[numthreads(1, 1, 1)]
void copyMember() { outMember[0].a = s.a; }

// TINT: OpEntryPoint GLCompute
