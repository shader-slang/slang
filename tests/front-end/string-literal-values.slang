// Test that string literal parsing produces correct values

//TEST:INTERPRET(filecheck=CHECK):  -warnings-as-errors all
void main()
{
    // ASCII characters

    printf("'%s'\n", " X]~");
    //CHECK: ' X]~'
    // UNICODE characters

    printf("'%s'\n", "ä€☺􏿽");
    //CHECK: 'ä€☺􏿽'

    // Many single-character escapes
    printf("'%s'\n", "\'\"\?\n123");
    //CHECK: ''"?
    //CHECK: 123'


    // octal numbers
    printf("'%s'\n", "\110\145\154\154\157");
    //CHECK: 'Hello'

    // hex numbers
    printf("'%s'\n", "\x48\x65\x6C\x6C\x6F");
    //CHECK: 'Hello'

    // hex-encoded UTF-8
    printf("'%s'\n", "\xF0\x9F\x98\x8A");
    //CHECK: '😊'

    // unicode code points
    printf("'%s'\n", "\u{1F60A}\u00E4\u20AC\U0010FFFD");
    //CHECK: '😊ä€􏿽'

}
