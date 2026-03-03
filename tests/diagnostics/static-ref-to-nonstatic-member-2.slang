//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

struct Foo {
    int a;

    static int func() {
        // CHECK: ([[# @LINE+1]]): error {{.*}} static function cannot refer to non-static member `a` via `this`
        return this.a;
    }
}
