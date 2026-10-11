// A parameter has at most one reference access, so `__ref_readonly` and `__ref_writeonly`
// conflict with each other, with `__ref`, with themselves, and with `const` on a `__ref`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

void readOnlyThenRef(__ref_readonly __ref int x) {}
//CHECK:                            ^^^^^ E31202
//CHECK:                            ^^^^^ modifier '__ref' is redundant or conflicting with existing modifier '__ref_readonly'

void refThenReadOnly(__ref __ref_readonly int x) {}
//CHECK:                   ^^^^^^^^^^^^^^ E31202
//CHECK:                   ^^^^^^^^^^^^^^ modifier '__ref_readonly' is redundant or conflicting with existing modifier '__ref'

void readOnlyThenWriteOnly(__ref_readonly __ref_writeonly int x) {}
//CHECK:                                  ^^^^^^^^^^^^^^^ E31202
//CHECK:                                  ^^^^^^^^^^^^^^^ modifier '__ref_writeonly' is redundant or conflicting with existing modifier '__ref_readonly'

void readOnlyTwice(__ref_readonly __ref_readonly int x) {}
//CHECK:                          ^^^^^^^^^^^^^^ E31202
//CHECK:                          ^^^^^^^^^^^^^^ modifier '__ref_readonly' is redundant or conflicting with existing modifier '__ref_readonly'

void constThenReadOnly(const __ref_readonly int x) {}
//CHECK:                     ^^^^^^^^^^^^^^ E31202
//CHECK:                     ^^^^^^^^^^^^^^ modifier '__ref_readonly' is redundant or conflicting with existing modifier 'const'

[numthreads(1, 1, 1)]
void main()
{
}
