// A module without a `module` declaration is named after its file
// (`implicit_own_module`), and that name is not a qualifier either
// (shader-slang/slang#14063).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

float twice(float x) { return 2.0f * x; }

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform float* o)
{
    o[0] = implicit_own_module.twice(o[0]);
//CHECK:   ^^^^^^^^^^^^^^^^^^^ error E30015
//CHECK:   ^^^^^^^^^^^^^^^^^^^ undefined identifier 'implicit_own_module'.
//CHECK:   ^^^^^^^^^^^^^^^^^^^ 'implicit_own_module' names a module; module names cannot be used as qualifiers, types, or values
}
