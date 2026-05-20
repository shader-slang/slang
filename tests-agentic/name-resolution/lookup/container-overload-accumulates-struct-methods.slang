//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/name-resolution/lookup.md#container-level-overload-accumulation
//META: doc_section_digest=abaee536cf032e619ae16ca54b65e279f6271cecbdc254213692b2f9ca5f2f4e
//META: purpose=Same-named methods inside one struct container accumulate into a LookupResult via the _prevInContainerWithSameName chain; overload resolution then picks the candidate matching the argument type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Container-level overload accumulation also applies to type
// members. Two `run` overloads of `K` share the same name; both
// must be reachable through `k.run(...)` and overload resolution
// selects by argument type.

//TEST:INTERPRET(filecheck=CHECK):

struct K
{
    int run(int a) { return a * 2; }    // run(7)   -> 14
    int run(float a) { return int(a) + 10; } // run(4.0f) -> 14
}

void main()
{
    K k;
    // CHECK: r=28
    printf("r=%d\n", k.run(7) + k.run(4.0f));
}
