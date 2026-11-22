//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang glsl tests/diagnostics/cpp-style-trailing-this-const-glsl.slang -no-codegen

// GLSL diagnoses trailing `const` independently of the diagnostic that rejects a receiver-mode
// spelling on a free function.

void freeFunction() const
/*CHECK:
                    ^^^^^ trailing `const` is only allowed in the HLSL dialect
                    ^^^^^ C++-style trailing `const` requires the HLSL-flavored dialect (use a `.hlsl` file or pass `-lang hlsl`).
                    ^^^^^ receiver-mode specifier requires an instance receiver
                    ^^^^^ receiver-mode specifier 'const' is only allowed on an instance member that has a `this` parameter.
*/
{}
