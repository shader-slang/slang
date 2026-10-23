//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-lang hlsl -Gec

// With `-Gec`, the parser renames each global shader parameter and adds a mutable shadow
// variable under its original name. For a `cbuffer`, the `__transparent` modifier moves from
// the buffer variable to the shadow. The members of the `cbuffer` still conflict with other
// declarations of the same name (shader-slang/slang#13876).

int globalUniform;
/*CHECK:
    ^^^^^^^^^^^^^ see previous declaration of 'globalUniform'
*/

cbuffer CB
{
    int globalUniform;
/*CHECK:
        ^^^^^^^^^^^^^ conflicting declaration
        ^^^^^^^^^^^^^ declaration of 'globalUniform' conflicts with existing declaration
*/
    int unique;
}

// A declaration after the `cbuffer` is compared with the shadow's members as well, and the
// members of a second `cbuffer` are compared with those of the first.

cbuffer LaterCB
{
    int laterUniform;
/*CHECK:
        ^^^^^^^^^^^^ see previous declaration of 'laterUniform'
*/
    int sharedMember;
/*CHECK:
        ^^^^^^^^^^^^ see previous declaration of 'sharedMember'
*/
}

int laterUniform;
/*CHECK:
    ^^^^^^^^^^^^ conflicting declaration
    ^^^^^^^^^^^^ declaration of 'laterUniform' conflicts with existing declaration
*/

cbuffer SecondCB
{
    int sharedMember;
/*CHECK:
        ^^^^^^^^^^^^ conflicting declaration
        ^^^^^^^^^^^^ declaration of 'sharedMember' conflicts with existing declaration
*/
}
