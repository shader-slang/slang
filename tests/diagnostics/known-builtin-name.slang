//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

[KnownBuiltin("GeometryStreamAppen")]
//CHECK:      ^^^^^^^^^^^^^^^^^^^^^ unknown builtin name
//CHECK:      ^^^^^^^^^^^^^^^^^^^^^ unknown builtin name 'GeometryStreamAppen'
void misspelledBuiltin() {}

// COUNT terminates the compiler's enum; it does not identify a builtin declaration.
[KnownBuiltin("COUNT")]
//CHECK:      ^^^^^^^ unknown builtin name
//CHECK:      ^^^^^^^ unknown builtin name 'COUNT'
void sentinelIsNotBuiltin() {}
