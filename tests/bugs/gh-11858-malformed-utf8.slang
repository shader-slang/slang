// Note: this is a simple test instead of a diagnostic test, since
// some UTF-8 errors get lost in communication with the test server

//TEST:SIMPLE(filecheck=CHECK): -target spirv -no-codegen

/* Ö */
//CHECK: invalid UTF-8 byte sequence: 0xD6


[numthreads(1,1,1)]
void main()
{
    const char *str = "å";
//CHECK: invalid UTF-8 byte sequence: 0xE5

    int ch = 'ü';
//CHECK: invalid UTF-8 byte sequence: 0xFC

    int brokenÏdentifier = 123;
//CHECK: invalid UTF-8 byte sequence: 0xCF

    if (true)
// Ã
        return;
//CHECK: invalid UTF-8 byte sequence: 0xC3
}

//CHECK: invalid UTF-8 byte sequence: 0xF0
// Note! Do not add newline on the last line
ð