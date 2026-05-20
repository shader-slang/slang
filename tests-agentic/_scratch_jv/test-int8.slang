//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    int8_t a = int8_t(-128);
    int8_t b = int8_t(127);
    // CHECK: min=-128
    printf("min=%d\n", int(a));
    // CHECK: max=127
    printf("max=%d\n", int(b));
}
