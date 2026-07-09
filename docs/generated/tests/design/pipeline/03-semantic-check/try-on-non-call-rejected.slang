//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
//META: purpose=Expression checking rejects a `try` clause applied to a non-call expression (E30090).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `try` is only valid as a prefix on a function call or operator
// invocation; applying it to a literal is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int main() throws int { int v = try 5; return v; }
//CHECK:                        ^^^ expression in 'try' must be a call
//CHECK:                        ^^^ expression in a 'try' clause must be a call to a function or operator overload.
// CHECK: E30090
