//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage fragment -entry fragmentMain

// Two `out` parameters declared `SV_Target0` overlap at location 0.

void fragmentMain(out float4 a : SV_Target0, out float4 b : SV_Target0)
//CHECK:                                                ^ overlapping varying locations
//CHECK:                                                ^ varying 'b' overlaps the location of 'a'
{
    a = float4(1);
    b = float4(2);
}
