//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[mutating]
/*CHECK:
 ^^^^^^^^ receiver-mode specifier requires an instance receiver
 ^^^^^^^^ receiver-mode specifier 'mutating' is only allowed on an instance member that has a `this` parameter.
*/
void freeFunction() {}

struct Value
{
    [constref]
    /*CHECK:
     ^^^^^^^^ receiver-mode specifier requires an instance receiver
     ^^^^^^^^ receiver-mode specifier 'constref' is only allowed on an instance member that has a `this` parameter.
    */
    static void staticMethod() {}

    [__ref]
    /*CHECK:
     ^^^^^ receiver-mode specifier requires an instance receiver
     ^^^^^ receiver-mode specifier '__ref' is only allowed on an instance member that has a `this` parameter.
    */
    __init() {}
}

class Reference
{
    static const int elementCount = 4;

    [mutating]
    /*CHECK:
     ^^^^^^^^ receiver-mode specifier is not allowed on a class member
     ^^^^^^^^ receiver-mode specifier 'mutating' is not allowed on a class member; a class receiver always uses the `in` parameter-passing mode.
    */
    void method(int[this.elementCount] values) {}

    [constref]
    /*CHECK:
     ^^^^^^^^ receiver-mode specifier is not allowed on a class member
     ^^^^^^^^ receiver-mode specifier 'constref' is not allowed on a class member; a class receiver always uses the `in` parameter-passing mode.
    */
    void borrowedMethod() {}

    [__ref]
    /*CHECK:
     ^^^^^ receiver-mode specifier is not allowed on a class member
     ^^^^^ receiver-mode specifier '__ref' is not allowed on a class member; a class receiver always uses the `in` parameter-passing mode.
    */
    void referenceMethod() {}

    [nonmutating]
    /*CHECK:
     ^^^^^^^^^^^ receiver-mode specifier is not allowed on a class member
     ^^^^^^^^^^^ receiver-mode specifier 'nonmutating' is not allowed on a class member; a class receiver always uses the `in` parameter-passing mode.
    */
    void nonMutatingMethod() {}
}

class ReferenceWithSetters
{
    int storage;

    property int throughExplicitThis
    {
        get { return storage; }
        set(int newValue) { this.storage = newValue; }
    }

    property int throughLookup
    {
        get { return storage; }
        set(int newValue) { storage = newValue; }
    }

    __subscript(int index) -> int
    {
        get { return storage; }
        set(int newValue)
        {
            this.storage = newValue;
            storage = newValue;
        }
    }
}

void useClassSetters()
{
    let instance = new ReferenceWithSetters();
    instance.throughExplicitThis = 1;
    instance.throughLookup = 2;
    instance[0] = 3;
}
