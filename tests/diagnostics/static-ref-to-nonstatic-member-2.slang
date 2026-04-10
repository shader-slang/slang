//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Foo {
    int a;

    static int func() {
        return this.a;
//CHECK:            ^ static function cannot refer to non-static member via `this`
//CHECK:            ^ static function cannot refer to non-static member `a` via `this`
    }
}
