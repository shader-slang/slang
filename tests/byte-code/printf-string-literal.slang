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
    // Mix a string-literal %s with a non-string %d in one printf so both copy-size
    // branches fire in a single Print: the literal must resolve and the int must not
    // be over-copied as a pointer. Also place a literal in a non-first vararg slot.
    printf("%s=%d\n", "k", 5);
    printf("%d %s\n", 1, "x");
}

// CHECK: lit
// CHECK: tern
// CHECK: str
// CHECK: nat
// CHECK: k=5
// CHECK: 1 x

// BC: func main
// BC: print {{.*}}str:"lit"
// BC: ret
