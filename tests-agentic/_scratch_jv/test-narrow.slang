//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    uint v = 0xFF03u;
    uint8_t low = uint8_t(v);
    // CHECK: low=3
    printf("low=%d\n", int(low));
}
