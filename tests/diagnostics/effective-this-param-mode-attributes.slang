//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[mutating]
/*CHECK:
 ^^^^^^^^ attribute requires an effective `this` parameter
 ^^^^^^^^ attribute 'mutating' is only allowed on a declaration with an effective `this` parameter.
*/
void freeFunction() {}

struct Value
{
    [constref]
    /*CHECK:
     ^^^^^^^^ attribute requires an effective `this` parameter
     ^^^^^^^^ attribute 'constref' is only allowed on a declaration with an effective `this` parameter.
    */
    static void staticMethod() {}

    [__ref]
    /*CHECK:
     ^^^^^ attribute requires an effective `this` parameter
     ^^^^^ attribute '__ref' is only allowed on a declaration with an effective `this` parameter.
    */
    __init() {}
}

class Reference
{
    [mutating]
    /*CHECK:
     ^^^^^^^^ attribute cannot change a class member's effective `this` parameter
     ^^^^^^^^ attribute 'mutating' is not allowed on a class member; the effective `this` parameter of a class member is always passed by value.
    */
    void method() {}
}
