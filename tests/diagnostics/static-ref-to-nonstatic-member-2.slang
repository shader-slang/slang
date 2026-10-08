//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Foo {
    int a;

    static int func() {
        return this.a;
//CHECK:       ^^^^ 'this' used in callable without an instance receiver
//CHECK:       ^^^^ 'this' expression cannot be used in a callable that has no instance receiver
    }
}
