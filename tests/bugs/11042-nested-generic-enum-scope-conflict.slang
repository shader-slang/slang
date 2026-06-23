// Dedicated diagnostic test for enumeration scope conflict on an enum
// declared as a member of a generic parent. This exercises the second
// branch of parseOptGenericDecl, where genericParent is non-null because
// the enclosing scope's container is a GenericDecl (rather than because
// the enum itself has its own generic parameters).
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct GenericContainer<T>
{
    [UnscopedEnum]
/*CHECK:
     ^^^^^^^^^^^^ duplicate modifier
     ^^^^^^^^^^^^ modifier 'UnscopedEnum' is redundant or conflicting with existing modifier 'class'
*/
    enum class NestedEnum
    {
    }
}

[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
}
