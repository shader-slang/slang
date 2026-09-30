//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Regression guard for https://github.com/shader-slang/slang/issues/10641.
//
// An undefined name passed to an `out`, `inout` or `groupshared` parameter reaches the checks that
// ask which storage an argument names (warning 30709, error 30711). Error recovery leaves such an
// argument with an error type and no declaration, and the checks must report nothing beyond the
// undefined names themselves; the same holds for group-shared storage of an undefined type. An
// element of an undefined name reaches the parameter through the implicit cast that coercion adds,
// and gets the same l-value error an `inout` argument would, but no E30711. The test is exhaustive,
// so any extra diagnostic, or a crash, fails it.

void takesInOut(inout Undefined x) {}
//diag:               ^^^^^^^^^ undefined identifier 'Undefined'.
//diag:               ^^^^^^^^^ undefined identifier
void takesOut(out Undefined x) {}
//diag:           ^^^^^^^^^ undefined identifier 'Undefined'.
//diag:           ^^^^^^^^^ undefined identifier
void takesShared(groupshared Undefined x) {}
//diag:                      ^^^^^^^^^ undefined identifier 'Undefined'.
//diag:                      ^^^^^^^^^ undefined identifier
void takesSharedInt(groupshared int x) {}

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
    takesSharedInt(undefinedArray[0]);
//diag:            ^^^^^^^^^^^^^^ undefined identifier 'undefinedArray'.
//diag:            ^^^^^^^^^^^^^^ undefined identifier
//diag:                          ^ argument passed to parameter '0' must be l-value.
//diag:                          ^ argument must be l-value
//diag:                          ^ argument was implicitly cast from 'error' to 'int', and Slang does not support using an implicit cast as an l-value with a reference
}

