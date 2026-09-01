//TEST:INTERPRET(filecheck=CHECK):

// Test recursive Optional<Optional<T>> -> Optional<Optional<U>> coercion

void main()
{
    // Nested some/some: Optional<Optional<int>> -> Optional<Optional<float>>
    Optional<int> inner = 5;
    Optional<Optional<int>> nested = inner;
    Optional<Optional<float>> coerced = nested;

    // CHECK: has-outer
    if (coerced.hasValue)
        printf("has-outer\n");
    // CHECK: has-inner
    if (coerced.hasValue && coerced.value.hasValue)
        printf("has-inner\n");
    // CHECK: 5.0
    if (coerced.hasValue && coerced.value.hasValue)
        printf("%.1f\n", coerced.value.value);

    // Outer-none propagates: Optional<Optional<int>>(none) -> Optional<Optional<float>>(none)
    Optional<Optional<int>> outerNone = none;
    Optional<Optional<float>> coercedOuterNone = outerNone;

    // CHECK: outer-none
    if (!coercedOuterNone.hasValue)
        printf("outer-none\n");
}
