//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage fragment -entry fragmentMain

// `val : SV_Target1` takes targets 1 and 2, so `Material : SV_Target2` overlaps it.

struct NestedReturn
{
    float4 debug1;
    float4 debug2;
};

struct Output
{
    float4 Diffuse : SV_Target0;
    NestedReturn val : SV_Target1;
    float4 Material : SV_Target2;
//CHECK:   ^^^^^^^^ overlapping varying locations
//CHECK:   ^^^^^^^^ varying 'Material' overlaps the location of 'val'
}

Output fragmentMain()
{
    return { float4(1), { float4(2), float4(2) }, float4(3) };
}
