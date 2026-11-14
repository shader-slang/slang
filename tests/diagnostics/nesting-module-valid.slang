//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// All declaration kinds allowed at module (file) scope.

struct S { int x; }
interface IFoo { void method(); }
enum E { A, B }
extension S { static int ext_s; }
void freeFunc() {}
int globalVar;
static int staticGlobalVar;
typealias MyInt = int;
typedef int MyInt2;
namespace NS {}
;
