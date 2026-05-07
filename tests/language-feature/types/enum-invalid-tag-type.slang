// enum-invalid-tag-type.slang

// Test that using an invalid underlying type (float) for an enum produces a diagnostic error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

enum Bad : float
//CHECK:   ^^^^^ invalid enum tag type
//CHECK:   ^^^^^ invalid tag type for 'enum': 'float'
{
    A,
    B
}
