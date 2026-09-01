//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#container-level-overload-accumulation
//META: doc_section_digest=cb7e905b70ffaa021ce6c0239be16311c8162dfb5969fc52c7a9db96f2aa7905
//META: purpose=Same-named decls inside one container do not shadow each other; they accumulate through the same-name chain into one overload set and resolution picks by argument type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a container hands lookup every same-named member, not
// just the last one declared. Both `run` overloads must reach the
// candidate set for the int call and the float call to select different
// bodies. CHECK asserts 14 + 14, a sum each body reaches by a different
// route, so a single surviving overload cannot produce it.

//TEST:INTERPRET(filecheck=CHECK):

struct K
{
    int run(int a) { return a * 2; }
    int run(float a) { return int(a) + 10; }
}

void main()
{
    K k;
    // CHECK: r=28
    printf("r=%d\n", k.run(7) + k.run(4.0f));
}
