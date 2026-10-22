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
