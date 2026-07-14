//TEST:INTERPRET(filecheck=CHECK):

enum ByteTag : uint8_t
{
    One = 1,
    Two = 2,
}

void main()
{
    int signedValue = 2;
    ByteTag fromSignedValue = ByteTag(signedValue);

    int16_t narrowSignedValue = 1;
    ByteTag fromNarrowSignedValue = (ByteTag)narrowSignedValue;

    printf("%u\n", uint(fromSignedValue));
    printf("%u\n", uint(fromNarrowSignedValue));
}

// CHECK: 2
// CHECK: 1
