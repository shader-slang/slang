//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage fragment -entry fragmentMain

// The result and an `out` parameter declared `SV_Target0` overlap at location 0.

float4 fragmentMain(out float4 a : SV_Target0) : SV_Target0
/*CHECK:
       ^^^^^^^^^^^^ overlapping varying locations
       ^^^^^^^^^^^^ varying 'fragmentMain' overlaps the location of 'a'
*/
{
    a = float4(1);
    return float4(2);
}
