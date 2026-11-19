// The name of the module being compiled is not a qualifier either
// (shader-slang/slang#14063).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

module ModSelf;

public float twice(float x) { return 2.0f * x; }

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform float* o)
{
    o[0] = ModSelf.twice(o[0]);
//CHECK:   ^^^^^^^ error E30015
//CHECK:   ^^^^^^^ undefined identifier 'ModSelf'.
//CHECK:   ^^^^^^^ 'ModSelf' names a module; module names cannot be used as qualifiers, types, or values

    // A misspelled module name is not answered with "did you mean 'ModSelf'?",
    // because lookup could not resolve that suggestion either.
    o[1] = ModSelff.twice(o[1]);
//CHECK:   ^^^^^^^^ error E30015
//CHECK:   ^^^^^^^^ undefined identifier 'ModSelff'.
}
