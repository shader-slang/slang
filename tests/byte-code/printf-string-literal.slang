//TEST:INTERPRET(filecheck=BC): -disasm
//TEST:INTERPRET(filecheck=CHECK):

int main()
{
    String stringValue = "string";
    NativeString nativeStringValue = "native";
    bool condition = true;

    // CHECK: literal
    printf("%s\n", "literal");
    // CHECK: string
    printf("%s\n", stringValue);
    // CHECK: native
    printf("%s\n", nativeStringValue);
    // CHECK: T
    printf("%s\n", condition ? "T" : "F");

    return 0;
}

// BC: func main
// BC: print
