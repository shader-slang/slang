//TEST:INTERPRET(filecheck=CHECK): -hgvm -legacy

// Engine options after the filename must remain arguments to the legacy program,
// including when a persistent test server handles the invocation.
int main(int argc, NativeString* argv)
{
    printf("%d\n", argc);
    return 0;
}

// CHECK: 3
