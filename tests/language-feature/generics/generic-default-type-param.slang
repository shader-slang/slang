//TEST:INTERPRET(filecheck=CHECK):

// Test that generic structs and functions with default type parameters
// work correctly when the type argument is omitted.

struct Container<T = float>
{
    T value;

    T get() { return value; }
}

T identity<T = int>(T v)
{
    return v;
}

void main()
{
    // Use Container without specifying type, should default to float.
    Container<> c1;
    c1.value = 3.5;
    printf("%f\n", c1.value);
    // CHECK: 3.5

    // Use Container with explicit type override.
    Container<int> c2;
    c2.value = 7;
    printf("%d\n", c2.value);
    // CHECK: 7

    // Use identity function without specifying type, should default to int.
    int r1 = identity<>(42);
    printf("%d\n", r1);
    // CHECK: 42

    // Use identity with explicit float type.
    float r2 = identity<float>(2.5);
    printf("%f\n", r2);
    // CHECK: 2.5
}
