//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    int* p = nullptr;
    int* q = nullptr;
    // CHECK: peqnull=1
    printf("peqnull=%d\n", (p == nullptr) ? 1 : 0);
    // CHECK: peqq=1
    printf("peqq=%d\n", (p == q) ? 1 : 0);
}
