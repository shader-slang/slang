//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Regression guard for https://github.com/shader-slang/slang/issues/10641.
//
// An undefined name passed to an `out`, `inout` or `groupshared` parameter reaches the checks that
// ask which storage an argument names (warning 30709, error 30711). Error recovery leaves such an
// argument with an error type and no declaration, and the checks must report nothing beyond the
// undefined names themselves; the same holds for group-shared storage of an undefined type. The
// test is exhaustive, so any extra diagnostic, or a crash, fails it.

void takesInOut(inout Undefined x) {}
//diag:               ^^^^^^^^^ undefined identifier 'Undefined'.
//diag:               ^^^^^^^^^ undefined identifier
void takesOut(out Undefined x) {}
//diag:           ^^^^^^^^^ undefined identifier 'Undefined'.
//diag:           ^^^^^^^^^ undefined identifier
void takesShared(groupshared Undefined x) {}
//diag:                      ^^^^^^^^^ undefined identifier 'Undefined'.
//diag:                      ^^^^^^^^^ undefined identifier

groupshared Undefined sharedOfUndefinedType;
//diag:     ^^^^^^^^^ undefined identifier 'Undefined'.
//diag:     ^^^^^^^^^ undefined identifier

[numthreads(1, 1, 1)]
void computeMain()
{
    takesInOut(undefinedVar);
//diag:        ^^^^^^^^^^^^ undefined identifier 'undefinedVar'.
//diag:        ^^^^^^^^^^^^ undefined identifier
    takesOut(undefinedVar);
//diag:      ^^^^^^^^^^^^ undefined identifier 'undefinedVar'.
//diag:      ^^^^^^^^^^^^ undefined identifier
    takesShared(undefinedVar);
//diag:         ^^^^^^^^^^^^ undefined identifier 'undefinedVar'.
//diag:         ^^^^^^^^^^^^ undefined identifier
    takesInOut(sharedOfUndefinedType);
}

