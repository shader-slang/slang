// Dedicated diagnostic test for enumeration scope conflict
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[UnscopedEnum]
/*CHECK:
 ^^^^^^^^^^^^ duplicate modifier
 ^^^^^^^^^^^^ modifier 'UnscopedEnum' is redundant or conflicting with existing modifier 'class'
*/
enum class TestEnum
{
    Value1,
}

[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
}
