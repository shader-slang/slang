// Dedicated diagnostic test for enumeration scope conflict
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[UnscopedEnum]
/*CHECK:
 ^^^^^^^^^^^^ conflicting attribute declaration
*/
enum class TestEnum
/*CHECK:
     ^^^^^ conflicting scoped/unscoped declarations for an enum
     ^^^^^ 'class' keyword for 'TestEnum' implies scoped enumeration
*/
{
    Value1,
}

RWStructuredBuffer<TestEnum> output;

[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    // NOTE: only enumerators of unscoped enumerations can be accessed directly
    output[0] = Value1;
}
