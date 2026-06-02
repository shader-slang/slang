//TEST:INTERPRET(filecheck=BC): -disasm
//TEST:INTERPRET(filecheck=CHECK):

// Regression test for shader-slang/slang#11399.
// The VM print handler copied `operand.size` bytes for each printf argument, but
// string-literal operands are emitted with size 0 (they point at a relocated
// `const char*` slot), so a `%s` literal/String/NativeString argument produced a
// zero-length buffer and segfaulted on dereference. A runtime NativeString
// (argv[i]) was a working-set operand of pointer size and was unaffected.

void main()
{
    printf("%s\n", "lit");
    int x = 1;
    printf("%s\n", x == 1 ? "tern" : "no");
    String s = "str";
    printf("%s\n", s);
    NativeString n = "nat";
    printf("%s\n", n);
}

// CHECK: lit
// CHECK: tern
// CHECK: str
// CHECK: nat

// BC: func main
// BC: print {{.*}}str:"lit"
// BC: ret
