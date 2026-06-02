//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:37:14+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-this.md#mutability
//META: doc_section_digest=2822c7f6b30708a773dbe170117103457227a510de3a625a9ca4bed55f2b62fe
//META: purpose=Calling a `[mutating]` member function on an r-value (temporary) is rejected because the implicit `this` argument must be an l-value.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "If a [mutating] instance is being called, the argument for the implicit
// `this` parameter must be an l-value."  A call expression returning a struct is an r-value,
// so invoking a [mutating] method on it must be rejected at compile time (E30050).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct Counter
{
    int value;
    [mutating] void increment() { value = value + 1; }
    static Counter make(int v) { Counter c; c.value = v; return c; }
}

void main()
{
    Counter.make(5).increment();
//CHECK:                     ^ mutating method cannot be called on immutable value
}
