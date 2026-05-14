//TEST:SIMPLE(filecheck=CHECK): -target llvm-host-ir -emit-cpu-via-llvm -whole-program

export __extern_cpp int main(int argc, Ptr<NativeString> argv)
{
    int a = 0;
    int b = 0;
    int c = 0;

    for (int i = 0; i < 8; ++i)
        a++;

    // CHECK: i32 8, i32 0, i32 0)
    printf("%d, %d, %d\n", a, b, c);
    return 0;
}
