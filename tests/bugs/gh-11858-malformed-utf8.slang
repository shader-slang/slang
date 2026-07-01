// Note: this is a simple test instead of a diagnostic test, since
// some UTF-8 errors get lost in communication with the test server

//TEST:SIMPLE(filecheck=CHECK): -target spirv -no-codegen

[numthreads(1,1,1)]
void main()
{
    if (true)
// Ã
        return;
}

//CHECK: invalid UTF-8 byte sequence: 0xC3
