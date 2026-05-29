//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Valid nestings (should NOT produce errors)
struct Outer { int x; }
struct Container {
    struct Inner { int y; }
    enum E { A, B }
    static int s;
    int field;
    void method() {}
    property int p { get { return 0; } }
    __subscript(int i) -> int { get { return 0; } }
}
interface IFoo {
    associatedtype T;
    void method();
    property int p { get; }
    __subscript(int i) -> int { get; }
}
enum ValidEnum : int { X = 1, Y = 2 }
extension Outer {
    static int ext_static;
    void ext_method() {}
    struct ExtInnerStruct { int z; }
    enum ExtInnerEnum { C, D }
    typealias MyType = int;
}
namespace ValidNS {}
typedef int MyInt;

// --- Invalid nesting: non-static field in extension ---

struct ForExt { int a; }
extension ForExt {
    int bad_field;
//CHECK:^^^^^^^^^ declaration not allowed in this context
//CHECK:^^^^^^^^^ non-static member variable declaration is not allowed inside extension.
}

// --- Invalid nesting: non-static field in interface ---

interface IBadField {
    int bad_iface_field;
//CHECK:^^^^^^^^^^^^^^^ declaration not allowed in this context
//CHECK:^^^^^^^^^^^^^^^ non-static member variable declaration is not allowed inside interface.
//CHECK:^^^^^^^^^^^^^^^ value requirement requires static const
//CHECK:^^^^^^^^^^^^^^^ requirement in the form of a simple value must be declared as 'static const'.
}
