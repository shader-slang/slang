// An `import` written in an `__include`d file of the module also names a
// module, so its spelling used as a qualifier gets the module-name note
// (shader-slang/slang#14063).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

module ModG;
__include "mod-g-impl";

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform float* o)
{
    o[0] = mod_b.twice(o[0]);
//CHECK:   ^^^^^ error E30015
//CHECK:   ^^^^^ undefined identifier 'mod_b'.
//CHECK:   ^^^^^ 'mod_b' names a module; module names cannot be used as qualifiers, types, or values

    // The unqualified call resolves through the included file's import.
    o[1] = twice(o[1]);
}
