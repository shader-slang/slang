//TEST:SIMPLE(filecheck=CHECK): -target llvm-host-ir -emit-cpu-via-llvm -whole-program

struct S2
{
    int b;
    int c;
};

struct S1
{
    int a;
    S2 s2;
};

export __extern_cpp int main(int argc, Ptr<NativeString> argv)
{
    int a = 0;
    int b = 0;
    int c = 0;

    for (int i = 0; i < 8; ++i)
        a++;

    // CHECK: i32 8, i32 0, i32 0)
    printf("%d, %d, %d\n", a, b, c);

    // The LLVM emitter should deconstruct structs when passing them to printf.
    S1 s1;
    s1.a = 1;
    s1.s2.b = 2;
    s1.s2.c = 3;
    // CHECK: i32 1, i32 2, i32 3)
    printf("%d, %d, %d\n", s1);
    return 0;
}
