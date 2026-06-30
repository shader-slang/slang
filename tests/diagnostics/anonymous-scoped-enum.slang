//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

enum class {
/*CHECK:
     ^^^^^ anonymous scoped enum is not allowed
     ^^^^^ 'class' keyword specifies a scoped enumeration
*/
    TestValue1, // not accessible
}
