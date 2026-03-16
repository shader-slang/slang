//TEST:INTERPRET(filecheck=CHECK):

// Test that generic structs with default value parameters
// work correctly when the value argument is omitted.

struct FixedArray<T, let N : int = 4>
{
    T data[N];

    int getCount() { return N; }
}

// A struct with only a default value parameter.
struct Summation<let N : int = 3>
{
    int compute()
    {
        int result = 0;
        for (int i = 0; i < N; i++)
            result += i + 1;
        return result;
    }
}

void main()
{
    // Use FixedArray with default N=4.
    FixedArray<int> arr;
    printf("%d\n", arr.getCount());
    // CHECK: 4

    // Use FixedArray with explicit N=2.
    FixedArray<int, 2> arr2;
    printf("%d\n", arr2.getCount());
    // CHECK: 2

    // Use Summation with default N=3 (1+2+3=6).
    Summation<> s1;
    printf("%d\n", s1.compute());
    // CHECK: 6

    // Use Summation with explicit N=5 (1+2+3+4+5=15).
    Summation<5> s2;
    printf("%d\n", s2.compute());
    // CHECK: 15
}
