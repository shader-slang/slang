//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#visibility-scopes
//META: doc_section_digest=e9c1d5ded205f4aeb978eada05ce7ffdf281151ff27ad0fb861b29efb600e2e2
//META: purpose=A `private` member is visible from the declaring type and from extensions of that same type, so an extension method can read it.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the positive half of the `private` row in the visibility table — the
// carve-out that admits extensions of the declaring type. This is the mirror of
// `visibility-private-member-not-accessible-rejected.slang`: there a free
// function in the same file is rejected, here an extension of the type is
// allowed, which is what shows the scope is the type rather than the file. The
// buffer store makes the read escape so it survives to the observed value.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct S
{
    private int secret;
    __init(int x) { secret = x; }
}

extension S
{
    int reveal() { return secret; }
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    S s = S(9);
    outputBuffer[0] = s.reveal();
}

//CHECK: 9
