//TEST:INTERPRET(hgvm):

void fail(String message)
{
    hgbc_asm
    {
        OpFail $message;
    };
}

[host]
void main()
{
    float2 imageSize = float2(8.0f, 12.0f);
    float2 center = imageSize * 0.5f;
    if (center.x != 4.0f || center.y != 6.0f)
        fail("HGVM vector-scalar broadcast produced the wrong value");

    float2 margin = 2.0f + imageSize;
    if (margin.x != 10.0f || margin.y != 14.0f)
        fail("HGVM scalar-vector broadcast produced the wrong value");

    bool2 isLarge = imageSize > 10.0f;
    if (isLarge.x || !isLarge.y)
        fail("HGVM comparison broadcast produced the wrong value");

    float2x2 matrix = float2x2(2.0f, 4.0f, 6.0f, 8.0f) / 2.0f;
    if (matrix[0][0] != 1.0f || matrix[1][1] != 4.0f)
        fail("HGVM matrix-scalar broadcast produced the wrong value");
}
