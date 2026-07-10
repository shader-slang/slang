//TEST:INTERPRET(filecheck=CHECK):

// Test implicit Optional<int> -> Optional<float> coercion

void main()
{
    // Test coercion of some value inline
    Optional<int> some = 7;
    Optional<float> f = some; // implicit coercion: Optional<int> -> Optional<float>
    // CHECK: some
    if (f.hasValue)
        printf("some\n");
    // CHECK: 7.0
    if (f.hasValue)
        printf("%.1f\n", f.value);

    // Test coercion of none inline
    Optional<int> none_val = none;
    Optional<float> none_f = none_val; // implicit coercion: Optional<int>(none) -> Optional<float>
    // CHECK: none
    if (!none_f.hasValue)
        printf("none\n");
}
