// Using the name of an imported module as a qualifier or a type is reported as
// an undefined identifier, with a note that module names cannot be used in code
// (shader-slang/slang#14063), rather than as an inaccessible declaration.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

import mod_a;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform float* o)
{
    o[0] = ModA.twice(o[0]);
//CHECK:   ^^^^ error E30015
//CHECK:   ^^^^ undefined identifier 'ModA'.
//CHECK:   ^^^^ 'ModA' names a module; module names cannot be used as qualifiers, types, or values

    ModA unusedVar;
/*CHECK:
    ^^^^ error E30015
    ^^^^ undefined identifier 'ModA'.
    ^^^^ 'ModA' names a module; module names cannot be used as qualifiers, types, or values
*/

    // The spelling used by the `import` also names the module.
    o[3] = mod_a.twice(o[3]);
//CHECK:   ^^^^^ error E30015
//CHECK:   ^^^^^ undefined identifier 'mod_a'.
//CHECK:   ^^^^^ 'mod_a' names a module; module names cannot be used as qualifiers, types, or values

    // The unqualified call still resolves through the import.
    o[1] = twice(o[1]);

    // A name that is not a module gets no note.
    o[2] = ModZ.twice(o[2]);
//CHECK:   ^^^^ error E30015
//CHECK:   ^^^^ undefined identifier 'ModZ'.
}
