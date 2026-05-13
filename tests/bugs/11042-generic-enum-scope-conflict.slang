// Dedicated diagnostic test for enumeration scope conflict
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// test the generic enums for completeness, even if they're not
// quite yet fully supported
[UnscopedEnum]
/*CHECK:
 ^^^^^^^^^^^^ duplicate modifier
 ^^^^^^^^^^^^ modifier 'UnscopedEnum' is redundant or conflicting with existing modifier 'class'
*/
enum class GenericEnum<T>
{
}

[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
}
