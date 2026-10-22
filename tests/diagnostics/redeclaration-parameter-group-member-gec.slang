//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-lang hlsl -Gec

// With `-Gec`, the compiler adds a mutable shadow variable for each global shader
// parameter, and the shadow of a `cbuffer` is the transparent variable. The members of
// the `cbuffer` still conflict with other declarations of the same name
// (shader-slang/slang#13876).

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
