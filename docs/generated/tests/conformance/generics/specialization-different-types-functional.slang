//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A generic struct can be independently specialized to multiple concrete types; each specialization is a distinct concrete type that can be instantiated and used independently.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the same generic struct specializes independently for each concrete type argument.
// `Pair<int>` and `Pair<float>` are each instantiated and swapped via a generic `swap<T>`;
// CHECK asserts the correct swapped values for both, proving distinct concrete types are produced.

//TEST:INTERPRET(filecheck=CHECK):

struct Pair<T>
{
    T first;
    T second;
}

func swap<T>(inout Pair<T> p)
{
    T tmp = p.first;
    p.first = p.second;
    p.second = tmp;
}

void main()
{
    Pair<int> ip;
    ip.first  = 10;
    ip.second = 20;
    swap(ip);
    // CHECK: 20 10
    printf("%d %d\n", ip.first, ip.second);

    Pair<float> fp;
    fp.first  = 1.5;
    fp.second = 3.5;
    swap(fp);
    // CHECK: 3 1
    printf("%d %d\n", (int)fp.first, (int)fp.second);
}
