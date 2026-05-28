//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    half oldHalf;
    uint packed0 = __packInterlockedAddF16Result(0x40003C00U, 0U, 2.0h, oldHalf);
    printf("lane0 %.1f %u\n", float(oldHalf), packed0);

    uint packed1 = __packInterlockedAddF16Result(0x40003C00U, 16U, 1.0h, oldHalf);
    printf("lane1 %.1f %u\n", float(oldHalf), packed1);
}

// CHECK: lane0 1.0 1073758720
// CHECK: lane1 2.0 1107311616
