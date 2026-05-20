//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    uint8_t a = uint8_t(255);
    uint8_t b = a + uint8_t(1);
    // CHECK: a=255
    printf("a=%d\n", int(a));
    // CHECK: b=0
    printf("b=%d\n", int(b));
}
