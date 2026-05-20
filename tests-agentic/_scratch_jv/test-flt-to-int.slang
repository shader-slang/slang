//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    float v = 3.7f;
    int i = int(v);
    // CHECK: i=3
    printf("i=%d\n", i);
    float nv = -3.7f;
    int ni = int(nv);
    // CHECK: ni=-3
    printf("ni=%d\n", ni);
}
