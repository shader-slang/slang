//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#declbase-modifiablesyntaxnode
//META: doc_section_digest=e171085f48e4577a56b6b6f39a0b22d92d9e9fdebe80185c8e6cda3a8029f6cf
//META: purpose=A declaration group is a ModifiableSyntaxNode, so modifiers written once in front of a multi-name declaration apply to every name the group declares.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the modifiers on a grouped declaration reach all of
// its names. `static const int P = 3, Q = 4;` writes `static const`
// once; both P and Q must therefore be usable as module-scope
// compile-time constants (slangi rejects non-const module globals, so
// the program only runs at all if `const` reached Q as well as P).
// A grouped struct field declaration is checked the same way.

//TEST:INTERPRET(filecheck=CHECK):

static const int P = 3, Q = 4;

struct V { int x, y; };

void main()
{
    V v;
    v.x = P;
    v.y = Q;
    //CHECK: v=3,4
    printf("v=%d,%d\n", v.x, v.y);
    //CHECK: pq=7
    printf("pq=%d\n", P + Q);
}
