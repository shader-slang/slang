//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    Optional<Optional<int>> a = Optional<int>(7);
    Optional<Optional<int>> b = none;
    // CHECK: outerHas=1
    printf("outerHas=%d\n", a.hasValue ? 1 : 0);
    // CHECK: innerHas=1
    printf("innerHas=%d\n", a.value.hasValue ? 1 : 0);
    // CHECK: innerV=7
    printf("innerV=%d\n", a.value.value);
    // CHECK: bHas=0
    printf("bHas=%d\n", b.hasValue ? 1 : 0);
}
