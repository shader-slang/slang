//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    uint8_t a = uint8_t(0);
    uint8_t b = uint8_t(255);
    uint8_t c = uint8_t(b + uint8_t(1));
    // CHECK: a=0
    printf("a=%d\n", int(a));
    // CHECK: b=255
    printf("b=%d\n", int(b));
    // CHECK: c=0
    printf("c=%d\n", int(c));
}
