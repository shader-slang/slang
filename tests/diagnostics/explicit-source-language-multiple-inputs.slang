// An explicitly requested translation-unit language intentionally takes precedence over every
// primary file's extension. Warnings are errors here so that both file orders verify that this
// deliberate override is silent.

//TEST:SIMPLE_EX: -lang slang tests/diagnostics/explicit-source-language-multiple-inputs.slang tests/diagnostics/explicit-source-language-multiple-inputs.vert -no-codegen -warnings-as-errors all
//TEST:SIMPLE_EX: -lang slang tests/diagnostics/explicit-source-language-multiple-inputs.vert tests/diagnostics/explicit-source-language-multiple-inputs.slang -no-codegen -warnings-as-errors all

void fromSlangFile() {}
