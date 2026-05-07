// Dedicated diagnostic test for enumeration scope conflict
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// test the generic enums for completeness, even if they're not
// quite yet fully supported
[UnscopedEnum]
/*CHECK:
 ^^^^^^^^^^^^ 'UnscopedEnum' specifies an unscoped enumeration
*/
enum class GenericEnum<T>
/*CHECK:
     ^^^^^ conflicting scoped/unscoped declarations for an enum
     ^^^^^ 'class' keyword for 'GenericEnum' specifies a scoped enumeration
*/
{
}

[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
}
