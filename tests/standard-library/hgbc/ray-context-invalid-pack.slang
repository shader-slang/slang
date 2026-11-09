// DIAGNOSTIC_TEST:SIMPLE(filecheck=MISSING): -target hgbc -entry missing
// DIAGNOSTIC_TEST:SIMPLE(filecheck=DUPLICATE): -target hgbc -entry duplicate
// DIAGNOSTIC_TEST:SIMPLE(filecheck=EMPTY): -target hgbc -entry empty
// DIAGNOSTIC_TEST:SIMPLE(filecheck=MISSING): -target hgbc -entry missing -g2
// DIAGNOSTIC_TEST:SIMPLE(filecheck=DUPLICATE): -target hgbc -entry duplicate -g2

import slang.pipeline_rt;

struct PrimaryRay : IRayType
{
    uint value;
}
struct ShadowRay : IRayType
{
    uint value;
}

[host]
void missing()
{
    print(rayTypeIndex<ShadowRay, PrimaryRay>());
}

[host]
void duplicate()
{
    HWRaytracingContext<PrimaryRay, PrimaryRay> duplicate = {};
}

[host]
void empty()
{
    HWRaytracingContext<> empty = {};
}

// MISSING: type is absent from the pack in __index_of_type_in_pack
// DUPLICATE: type occurs more than once in the pack in __index_of_type_in_pack
// EMPTY: A context requires between one and fifteen ray types.
