// Dedicated diagnostic test for enumeration scope conflict
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[UnscopedEnum]
/*CHECK:
 ^^^^^^^^^^^^ 'UnscopedEnum' specifies an unscoped enumeration
*/
enum class TestEnum
/*CHECK:
     ^^^^^ conflicting scoped/unscoped declarations for an enum
     ^^^^^ 'class' keyword for 'TestEnum' specifies a scoped enumeration
*/
{
    Value1,
}

[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
}
