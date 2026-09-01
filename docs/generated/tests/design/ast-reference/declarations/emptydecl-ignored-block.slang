//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
//META: purpose=An `__ignored_block { ... }` is skipped wholesale and leaves behind an empty declaration.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the contents of an ignored block are consumed without being
// parsed as Slang. The block contains text that is not a valid declaration; if
// it were parsed the file would not compile. The CHECK asserts the entry
// point's output, proving the block was skipped and left nothing behind.

//TEST:INTERPRET(filecheck=CHECK):

__ignored_block { this text is not a slang declaration at all (a, b) }

void main()
{
    //CHECK: ok
    printf("ok\n");
}
