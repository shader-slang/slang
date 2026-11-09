// TEST:INTERPRET(hgvm,filecheck=CHECK):

import slang.pipeline_rt;

struct PrimaryRay : IRayType
{
    uint value;
}
struct ShadowRay : IRayType
{
    uint value;
}
struct AnotherRay : IRayType
{
    float value;
}

[host]
void main()
{
    HWRaytracingContext<PrimaryRay, ShadowRay, AnotherRay> context = {};
    print(rayTypeIndex<PrimaryRay, PrimaryRay, ShadowRay, AnotherRay>());
    print(rayTypeIndex<ShadowRay, PrimaryRay, ShadowRay, AnotherRay>());
    print(rayTypeIndex<AnotherRay, PrimaryRay, ShadowRay, AnotherRay>());
}

// CHECK: 0
// CHECK: 1
// CHECK: 2
