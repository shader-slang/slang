//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=045c5379570a64224b0f0d784c24bc57410eef152e105148ecb4e662cb3f9c88
//META: purpose=The pointer-width suffix `z` types an integer literal as `intptr_t`, or `uintptr_t` when `u` is also present, and for a non-decimal literal it does so without consulting the magnitude, so `0xFFz` is `intptr_t`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the pointer-width arm of integer-literal typing with an overload
// probe: the literal's static type is not otherwise visible from source, but it
// decides which `probe` overload is selected, so the returned tag reports it.
// `0xFFz` is the interesting cell — a hex literal small enough that a
// magnitude-driven rule would have picked a narrow type — and its `intptr_t`
// tag is what shows the suffix alone decided. The `u` part is tested in both
// orderings to confirm it composes with `z` rather than replacing it.

//TEST:INTERPRET(filecheck=CHECK):

int probe(intptr_t) { return 1; }
int probe(uintptr_t) { return 2; }
int probe(int) { return 3; }
int probe(uint) { return 4; }
int probe(int64_t) { return 5; }
int probe(uint64_t) { return 6; }

void main()
{
    // Hex base: magnitude is not consulted, so the small value 0xFF is still
    // pointer-width signed rather than `int`.
    //CHECK: hexz=1
    printf("hexz=%d\n", probe(0xFFz));

    //CHECK: decz=1
    printf("decz=%d\n", probe(255z));

    //CHECK: uz=2
    printf("uz=%d\n", probe(255uz));

    //CHECK: zu=2
    printf("zu=%d\n", probe(255zu));
}
