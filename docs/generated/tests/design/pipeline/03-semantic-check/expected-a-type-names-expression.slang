//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=1690bc07bc351f6d57ef0cbfbdf90dd7a3a711d8e434d872c178e166712d1f85
//META: purpose=When an expression in type position does not denote a type, the diagnostic is built from the expression's actual type and the referenced name (E30060).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the diagnostic names the offending construct rather than
// reporting a bare "syntax error". `notAType` is a global `int` variable used
// where a type is expected; the message has to reach for both its resolved
// type and its name. CHECK pins E30060 and the rendered message text carrying
// both pieces.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int notAType = 3;

void main()
{
    notAType v;
//CHECK: E30060
//CHECK: got a 'int' for 'notAType'
}
