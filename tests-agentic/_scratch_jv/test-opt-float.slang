//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    Optional<float> a = 3.5f;
    Optional<bool> b = true;
    // CHECK: aHas=1
    printf("aHas=%d\n", a.hasValue ? 1 : 0);
    // CHECK: aV=3
    printf("aV=%d\n", int(a.value));
    // CHECK: bV=1
    printf("bV=%d\n", b.value ? 1 : 0);
}
