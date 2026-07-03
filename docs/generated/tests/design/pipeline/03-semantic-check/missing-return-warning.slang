//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
//META: purpose=Statement checking detects functions that fall off the end without returning and emits the missing-return warning (41010).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// slang-check-stmt.cpp is documented to validate return paths.
// A non-void function with no `return` statement triggers the
// missing-return warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int foo()
/*CHECK:
    ^^^ non-void function does not return in all cases
*/
// CHECK: W41010
{
    int x = 5;
}

void main() { int z = foo(); }
