// A parameter has at most one reference access, so `__ref_readonly` and `__ref_writeonly`
// conflict with each other, with `__ref`, with themselves, and with `const`, because `const __ref`
// is the legacy spelling of `__ref_readonly`. That spelling, in either order, remains valid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute -experimental-feature

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

void constThenWriteOnly(const __ref_writeonly int x) {}
//CHECK:                      ^^^^^^^^^^^^^^^ E31202
//CHECK:                      ^^^^^^^^^^^^^^^ modifier '__ref_writeonly' is redundant or conflicting with existing modifier 'const'

void readOnlyThenConst(__ref_readonly const int x) {}
//CHECK:                              ^^^^^ E31202
//CHECK:                              ^^^^^ modifier 'const' is redundant or conflicting with existing modifier '__ref_readonly'

void writeOnlyThenConst(__ref_writeonly const int x) {}
//CHECK:                                ^^^^^ E31202
//CHECK:                                ^^^^^ modifier 'const' is redundant or conflicting with existing modifier '__ref_writeonly'

void constRefThenReadOnly(const __ref __ref_readonly int x) {}
//CHECK:                              ^^^^^^^^^^^^^^ E31202
//CHECK:                              ^^^^^^^^^^^^^^ modifier '__ref_readonly' is redundant or conflicting with existing modifier '__ref'
//CHECK:                              ^^^^^^^^^^^^^^ E31202
//CHECK:                              ^^^^^^^^^^^^^^ modifier '__ref_readonly' is redundant or conflicting with existing modifier 'const'

void legacyConstRef(const __ref int x) {}

void legacyRefConst(__ref const int x) {}

[numthreads(1, 1, 1)]
void main()
{
}
