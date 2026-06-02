//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T16:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/types-array.md#declaration-syntax
//META: doc_section_digest=ad7e70e01944de764a4e5bc82497a352737e4133daa640a2bfae8511b826b4e5
//META: purpose=Per Remark 4, arrays in Slang are passed by value — mutating an array parameter inside a callee does NOT propagate back to the caller's array.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc explicitly notes "Unlike in C and C++, array types in
// Slang do not decay to pointer types... array objects are always
// passed as values in assignment and function calls, similar to
// `std::array`." Verify a function that overwrites its array
// parameter does NOT affect the caller's array.

//TEST:INTERPRET(filecheck=CHECK):

void mutate(int x[3])
{
    x[0] = 99;
    x[1] = 99;
    x[2] = 99;
}

void main()
{
    int caller[3] = { 1, 2, 3 };
    mutate(caller);
    //CHECK: caller=1 2 3
    printf("caller=%d %d %d\n", caller[0], caller[1], caller[2]);
}
