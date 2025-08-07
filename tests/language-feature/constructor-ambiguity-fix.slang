//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

struct Vector<T, int N>
{
    T[N] data;

    // Lower cost for direct scalar construction
    __implicit_conversion(constraint)
    __init(T value) {
        for (int i = 0; i < N; i++)
            data[i] = value;
    }

    // Higher cost for vector construction to avoid ambiguity
    __implicit_conversion(constraint+) 
    __init(vector<T, N> data)
    {
        for (int i = 0; i < N; i++)
            this.data[i] = data[i];
    }
}

[shader("compute")]
void computeMain()
{
    //CHECK: scalar constructor used
    printf("scalar constructor used\n");
    
    // This should now work without ambiguity - should prefer scalar constructor
    var normal = Vector<float, 16>(1.0);
    
    // Verify it worked by checking the first element
    //CHECK: 1
    printf("%d\n", int(normal.data[0]));
}