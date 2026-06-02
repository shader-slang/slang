//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Negative: a file that `#include`s itself renders E15302 with the documented "cyclic include" message and the offending path interpolated.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin `cyclic-include` (15302) to its rendered code by triggering
// a self-include via the file's own name. The `~path` interpolation
// inserts the verbatim include argument into the rendered message.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl
#include "error-code-cyclic-include.slang"
void main() {}

// CHECK: E15302
// CHECK: cyclic `#include` of file
// CHECK: error-code-cyclic-include.slang
