//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/name-resolution/index.md#flow-diagram
//META: doc_section_digest=5bf0b1723f5c95f0efe882446074467f222ba31a7cb99b373e5cfcaa35edc75e
//META: purpose=The index doc states that phases interleave; shadowing is enforced DURING the scope walk, so an inner-scope decl of the same name short-circuits before the outer decl is ever returned to overload resolution.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The index doc notes: "In the actual compiler some of these
// phases are interleaved -- for example, lookup.md describes how
// shadowing is enforced DURING the scope walk." The
// cross-cutting consequence visible at the index level is that
// the inner-scope name wins observably, with no ambiguity
// diagnostic, even though an outer-scope decl of the same name
// exists and would be reachable by a continued walk.

//TEST:INTERPRET(filecheck=CHECK):

static const int answer = 100;

int report()
{
    // Local `answer` shadows the file-scope one. Because
    // shadowing is decided during the walk (not as a
    // post-processing step that would otherwise produce an
    // ambiguity), the inner name wins silently.
    int answer = 7;
    return answer;
}

void main()
{
    // CHECK: r=7
    printf("r=%d\n", report());
}
