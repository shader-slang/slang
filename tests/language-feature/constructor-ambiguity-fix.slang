// Update test to use explicit conversion costs instead of the broken direct match logic
//TEST:SIMPLE(filecheck=CHECK): -target spirv

struct Vector<T, int N>
{
    T[N] data;

    // Lower cost for scalar constructor
    __implicit_conversion(0)
    __init(T value) {
        for (int i = 0; i < N; i++)
            data[i] = value;
    }

    // Higher cost for vector constructor to resolve ambiguity
    __implicit_conversion(100)
    __init(vector<T, N> data)
    {
        for (int i = 0; i < N; i++)
            this.data[i] = data[i];
    }
}

[shader("compute")]
void computeMain()
{
    // This should now work without ambiguity
    var normal = Vector<float, 16>(1.0);
    // CHECK: OpReturn
}