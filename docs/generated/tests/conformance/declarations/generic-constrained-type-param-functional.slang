//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#generics
//META: doc_section_digest=de256a399516188e43bb96d93ea3c68437fd40807db23a0da48c05221ab85f6d
//META: purpose=A generic type parameter constrained with `: IFoo` allows the function to call interface members on that parameter.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "A clause like `T : IFoo` is used to introduce a generic type
// parameter `T` where the parameter is _constrained_ so that it must conform
// to the `IFoo` interface."

//TEST:INTERPRET(filecheck=CHECK):

interface IHasScore { int getScore(); }

int sumScores<T : IHasScore>(T a, T b) { return a.getScore() + b.getScore(); }

struct Player : IHasScore
{
    int score;
    int getScore() { return score; }
}

void main()
{
    Player p1, p2;
    p1.score = 30;
    p2.score = 20;
    // CHECK: 50
    printf("%d\n", sumScores(p1, p2));
}
