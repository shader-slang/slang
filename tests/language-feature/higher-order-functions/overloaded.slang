//TEST:INTERPRET(filecheck=CHECK):

func foo(f : functype (float) -> int) -> int{
    return f(0);
}

int bit<T>(T) {
    return 10;
}

int bit<T, let N : int>(vector<T, N>) {
    return 1;
}

int zit() {
    // even though foo is overloaded, we should still be able to infer that we want bit<T>
    // based on the parameter (expected) type.
    return foo(bit<float>);
}

void main()
{
    // CHECK: 10
    printf("%d\n", zit());
}