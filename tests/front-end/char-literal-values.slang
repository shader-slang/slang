// Test that character literal parsing produces correct values

//TEST:INTERPRET(filecheck=CHECK):  -warnings-as-errors all
void main()
{
    // ASCII characters

    printf("'0x%X'\n", ' ');
    //CHECK: '0x20'

    printf("'0x%X'\n", 'X');
    //CHECK: '0x58'

    printf("'0x%X'\n", ']');
    //CHECK: '0x5D'

    printf("'0x%X'\n", '~');
    //CHECK: '0x7E'

    // UNICODE characters

    printf("'0x%X'\n", 'ä');
    //CHECK: '0xE4'

    printf("'0x%X'\n", '€');
    //CHECK: '0x20AC'

    printf("'0x%X'\n", '☺');
    //CHECK: '0x263A'

    printf("'0x%X'\n", '􏿽');
    //CHECK: '0x10FFFD'

    // All defined single-character escapes

    printf("'0x%X'\n", '\'');
    //CHECK: '0x27'

    printf("'0x%X'\n", '\"');
    //CHECK: '0x22'

    printf("'0x%X'\n", '\\');
    //CHECK: '0x5C'

    printf("'0x%X'\n", '\?');
    //CHECK: '0x3F'

    printf("'0x%X'\n", '\a');
    //CHECK: '0x7'

    printf("'0x%X'\n", '\b');
    //CHECK: '0x8'

    printf("'0x%X'\n", '\f');
    //CHECK: '0xC'

    printf("'0x%X'\n", '\n');
    //CHECK: '0xA'

    printf("'0x%X'\n", '\r');
    //CHECK: '0xD'

    printf("'0x%X'\n", '\t');
    //CHECK: '0x9'

    printf("'0x%X'\n", '\v');
    //CHECK: '0xB'

    // octal numbers

    printf("'0x%X'\n", '\0');
    //CHECK: '0x0'

    printf("'0x%X'\n", '\10');
    //CHECK: '0x8'

    printf("'0x%X'\n", '\25');
    //CHECK: '0x15'

    printf("'0x%X'\n", '\77');
    //CHECK: '0x3F'

    printf("'0x%X'\n", '\123');
    //CHECK: '0x53'

    printf("'0x%X'\n", '\377');
    //CHECK: '0xFF'

    // hex numbers
    printf("'0x%X'\n", '\x0');
    //CHECK: '0x0'

    printf("'0x%X'\n", '\x12');
    //CHECK: '0x12'

    printf("'0x%X'\n", '\x7f');
    //CHECK: '0x7F'

    printf("'0x%X'\n", '\x7F');
    //CHECK: '0x7F'

    printf("'0x%X'\n", '\xFEDC');
    //CHECK: '0xFEDC'

    printf("'0x%X'\n", '\xFEDC1234');
    //CHECK: '0xFEDC1234'

    printf("'0x%X'\n", '\xfFfFfFfF');
    //CHECK: '0xFFFFFFFF'

    // hex numbers in curly braces
    printf("'0x%X'\n", '\x{0}');
    //CHECK: '0x0'

    printf("'0x%X'\n", '\x{12}');
    //CHECK: '0x12'

    printf("'0x%X'\n", '\x{7f}');
    //CHECK: '0x7F'

    printf("'0x%X'\n", '\x{7F}');
    //CHECK: '0x7F'

    printf("'0x%X'\n", '\x{FEDC}');
    //CHECK: '0xFEDC'

    printf("'0x%X'\n", '\x{FEDC1234}');
    //CHECK: '0xFEDC1234'

    printf("'0x%X'\n", '\x{fFfFfFfF}');
    //CHECK: '0xFFFFFFFF'

    // UNICODE code points

    printf("'0x%X'\n", '\u0000');
    //CHECK: '0x0'

    printf("'0x%X'\n", '\u0012');
    //CHECK: '0x12'

    printf("'0x%X'\n", '\u1234');
    //CHECK: '0x1234'

    printf("'0x%X'\n", '\ufFfF');
    //CHECK: '0xFFFF'

    printf("'0x%X'\n", '\u{0}');
    //CHECK: '0x0'

    printf("'0x%X'\n", '\u{012}');
    //CHECK: '0x12'

    printf("'0x%X'\n", '\u{1234}');
    //CHECK: '0x1234'

    printf("'0x%X'\n", '\u{fFfF}');
    //CHECK: '0xFFFF'

    printf("'0x%X'\n", '\u{10ffff}');
    //CHECK: '0x10FFFF'

    printf("'0x%X'\n", '\u{ffffffff}');
    //CHECK: '0xFFFFFFFF'

    printf("'0x%X'\n", '\u{00fefefefe}');
    //CHECK: '0xFEFEFEFE'

    printf("'0x%X'\n", '\U00000012');
    //CHECK: '0x12'

    printf("'0x%X'\n", '\U00001234');
    //CHECK: '0x1234'

    printf("'0x%X'\n", '\U0000fFfF');
    //CHECK: '0xFFFF'

    printf("'0x%X'\n", '\U0010ffff');
    //CHECK: '0x10FFFF'

    printf("'0x%X'\n", '\Uffffffff');
    //CHECK: '0xFFFFFFFF'
}
