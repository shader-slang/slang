//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK: OpDecorate {{.*}} Location 0
// CHECK: OpDecorate {{.*}} Location 1

struct VSOut {
    Optional<float> a;
};

[shader("vertex")]
VSOut vertMain()
{
    VSOut o;
    o.a = 1.0;
    return o;
}