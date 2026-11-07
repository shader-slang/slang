//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std legacy -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2025 -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2026 -no-codegen

// Legacy implicit boxing must report the same unsupported operation as explicit `dyn`.
void ordinary(IDifferentiablePtrType value) {}

[Differentiable]
void parameter(IDifferentiablePtrType value)
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^ differentiation of existential box type 'dyn IDifferentiablePtrType' through IDifferentiablePtrType is not supported
{}
