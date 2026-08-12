//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=c0e97396ea139ceab54212053b6d443692d19f0faf2847c4dc505373ae8c4861
//META: purpose=With machine-readable diagnostics enabled the sink renders each diagnostic as a tab-separated `E<code> <severity> <file> <beginline> <begincol> <endline> <endcol> <message>` record.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the alternate rendering the sink offers to tools. Compiling with
// `-enable-machine-readable-diagnostics` replaces the caret presentation with one
// record per diagnostic; CHECK asserts the field order of that record for a
// known warning, matching code, severity word, this file's name, the begin and
// end line/column numbers and the message on a single line. FileCheck treats the
// separating tabs as whitespace, so the assertion is on field order, not on the
// exact separator.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -enable-machine-readable-diagnostics

RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int x = (1, 2);
    buf[0] = x;
}

// CHECK: E41024 warning {{.*}}machine-readable-diagnostic-record.slang {{[0-9]+}} {{[0-9]+}} {{[0-9]+}} {{[0-9]+}} comma operator used in expression
