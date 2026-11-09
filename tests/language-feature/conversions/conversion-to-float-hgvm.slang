//TEST:INTERPRET(hgvm):

void fail(String message)
{
    hgbc_asm
    {
        OpFail $message;
    };
}

[host]
float convertSigned(int value)
{
    return float(value);
}

[host]
float convertUnsigned(uint value)
{
    return float(value);
}

[host]
void main()
{
    if (convertSigned(-7) != -7.0)
        fail("Signed integer-to-float conversion produced the wrong value");
    if (convertUnsigned(0xffffffffu) != 4294967296.0)
        fail("Unsigned integer-to-float conversion produced the wrong value");
}
