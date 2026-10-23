//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// A member that repeats an earlier member of the same `cbuffer` is reported against that
// member only, not also against a declaration in the enclosing scope (shader-slang/slang#13876).
// The other diagnostics come from the constructor synthesized for the buffer's element struct,
// as for any struct with a repeated field.

static const int repeatedMember = 1;
/*CHECK:
                 ^^^^^^^^^^^^^^ see previous declaration of 'repeatedMember'
*/

cbuffer BufferWithRepeatedMember
/*CHECK:
        ^^^^^^^^^^^^^^^^^^^^^^^^ conflicting declaration
        ^^^^^^^^^^^^^^^^^^^^^^^^ declaration of 'repeatedMember' conflicts with existing declaration
        ^^^^^^^^^^^^^^^^^^^^^^^^ see previous declaration of 'repeatedMember'
        ^^^^^^^^^^^^^^^^^^^^^^^^ some of the above errors may have been produced while synthesizing method '$init'
*/
{
    int repeatedMember;
/*CHECK:
        ^^^^^^^^^^^^^^ conflicting declaration
        ^^^^^^^^^^^^^^ declaration of 'repeatedMember' conflicts with existing declaration
        ^^^^^^^^^^^^^^ see previous declaration of 'repeatedMember'
        ^^^^^^^^^^^^^^ ambiguous reference to 'repeatedMember'
        ^^^^^^^^^^^^^^ candidate: int SLANG_ParameterGroup_BufferWithRepeatedMember.repeatedMember
        ^^^^^^^^^^^^^^ candidate: int SLANG_ParameterGroup_BufferWithRepeatedMember.repeatedMember
*/
    int repeatedMember;
/*CHECK:
        ^^^^^^^^^^^^^^ conflicting declaration
        ^^^^^^^^^^^^^^ declaration of 'repeatedMember' conflicts with existing declaration
        ^^^^^^^^^^^^^^ ambiguous reference to 'repeatedMember'
        ^^^^^^^^^^^^^^ candidate: int SLANG_ParameterGroup_BufferWithRepeatedMember.repeatedMember
        ^^^^^^^^^^^^^^ candidate: int SLANG_ParameterGroup_BufferWithRepeatedMember.repeatedMember
*/
}
