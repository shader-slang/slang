// `countof` of a fixed-size array is its element count, independent of the
// element's byte size and of whether the element is a scalar, vector, or
// struct. The count must also fold for a generic array `T[N]` once `N` is a
// concrete constant after specialization. (shader-slang/slang#11317)

//TEST:INTERPRET(filecheck=CHECK):

struct Pixel
{
    float r;
    float g;
    float b;
}

int genericCount<let N : int>()
{
    float arr[N];
    return countof(arr);
}

void main()
{
    int a5[5];
    int a7[7];
    double d5[5];
    uint16_t h5[5];
    int2 v3[3];
    Pixel p3[3];
    static const int s5[5] = { 0, 0, 0, 0, 0 };

    // CHECK: int5=5
    printf("int5=%d\n", countof(a5));
    // CHECK: int7=7
    printf("int7=%d\n", countof(a7));
    // CHECK: dbl5=5
    printf("dbl5=%d\n", countof(d5));
    // CHECK: u16_5=5
    printf("u16_5=%d\n", countof(h5));
    // CHECK: int2x3=3
    printf("int2x3=%d\n", countof(v3));
    // CHECK: pixel3=3
    printf("pixel3=%d\n", countof(p3));
    // CHECK: static5=5
    printf("static5=%d\n", countof(s5));
    // CHECK: generic6=6
    printf("generic6=%d\n", genericCount<6>());
}
