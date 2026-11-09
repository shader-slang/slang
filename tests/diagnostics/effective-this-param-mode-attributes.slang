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

// h-slang keeps legacy class receiver attributes as no-ops on the by-value handle.
class Reference
{
    static const int elementCount = 4;

    [mutating]
    void method(int[this.elementCount] values) {}

    [constref]
    void borrowedMethod() {}

    [__ref]
    void referenceMethod() {}
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
