// Separators preserve integer-literal token boundaries: a `.x` swizzle still ends the
// integer, `0_` selects decimal (not octal), and separators may follow the radix prefix,
// end the literal, or repeat. `0x1e_5` stays a hexadecimal integer: `e` is a hex digit there, not an exponent.

//TEST:INTERPRET(filecheck=CHECK):  -warnings-as-errors all
void main()
{
    printf("%d\n", 1_0.x);
    // CHECK: 10
    printf("%d\n", 1_.x);
    // CHECK-NEXT: 1
    printf("%d\n", 0_7);
    // CHECK-NEXT: 7
    printf("%d\n", 0x_FF);
    // CHECK-NEXT: 255
    printf("%d\n", 0x1e_5);
    // CHECK-NEXT: 485
    printf("%d\n", 1__0);
    // CHECK-NEXT: 10
}
