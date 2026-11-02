//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/cpp-style-trailing-this-const-conflicts-hlsl.slang -no-codegen

// A declaration can have at most one receiver-mode spelling. Error recovery uses the first
// specifier, so both method bodies keep their explicitly nonmutating receiver after diagnosis.

struct Counter
{
    int value;

    [nonmutating]
    [nonmutating]
    /*CHECK:
     ^^^^^^^^^^^ duplicate modifier
     ^^^^^^^^^^^ modifier 'nonmutating' is redundant or conflicting with existing modifier 'nonmutating'
    */
    void repeatedAttribute()
    {
        value = 0;
//CHECK:^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
//CHECK:      ^ left of '=' is not an l-value
//CHECK:      ^ left of '=' is not an l-value.
    }

    [nonmutating]
    void attributeAndTrailingConst() const
    /*CHECK:
                                     ^^^^^ duplicate modifier
                                     ^^^^^ modifier 'const' is redundant or conflicting with existing modifier 'nonmutating'
    */
    {
        value = 0;
//CHECK:^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
//CHECK:      ^ left of '=' is not an l-value
//CHECK:      ^ left of '=' is not an l-value.
    }
};

// Requirement matching uses the effective receiver mode. A default HLSL method is mutating, so it
// cannot satisfy a requirement made non-mutating by trailing `const`.
interface INonmutatingReceiver
{
    void read() const;
//CHECK: ^^^^ see interface requirement declaration of 'read'
};

struct MutatingImplementation : INonmutatingReceiver
{
    void read() {}
//CHECK: ^^^^ interface requirement mismatch
//CHECK: ^^^^ member 'read' does not match interface requirement.
};

struct GenericReceiver
{
    int value;

    T read<T>(T result) const
    {
        value = 0;
//CHECK:^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
//CHECK:      ^ left of '=' is not an l-value
//CHECK:      ^ left of '=' is not an l-value.
        return result;
    }
};

struct SignatureDisplay
{
    int read(int value) const
//CHECK:^^^^ candidate: func SignatureDisplay.read(int) const -> int
    {
        return value;
    }

    T readGeneric<T>(T value) const
    /*CHECK:
      ^^^^^^^^^^^ candidate: func SignatureDisplay.readGeneric<int>(int) const -> int
      ^^^^^^^^^^^ candidate: func SignatureDisplay.readGeneric<T> const -> T
    */
    {
        return value;
    }

    func readModern(value: int) const -> int
//CHECK: ^^^^^^^^^^ candidate: func SignatureDisplay.readModern(int) const -> int
    {
        return value;
    }

    func readModernGeneric<T>(value: T) const -> T
//CHECK: ^^^^^^^^^^^^^^^^^ candidate: func SignatureDisplay.readModernGeneric<int>(int) const -> int
    {
        return value;
    }

    void call()
    {
        read();
//CHECK:    ^ not enough arguments to call
//CHECK:    ^ not enough arguments to call (got 0, expected 1)
        readGeneric<int>();
//CHECK:                ^ not enough arguments to call
//CHECK:                ^ not enough arguments to call (got 0, expected 1)
        readGeneric<int, int>(0);
//CHECK:^^^^^^^^^^^ too many arguments to call
//CHECK:^^^^^^^^^^^ too many arguments to call (got 2, expected 1)
        readModern();
//CHECK:          ^ not enough arguments to call
//CHECK:          ^ not enough arguments to call (got 0, expected 1)
        readModernGeneric<int>();
//CHECK:                      ^ not enough arguments to call
//CHECK:                      ^ not enough arguments to call (got 0, expected 1)
    }
};

// Receiver mode does not participate in overload identity. Trailing `const` therefore does not
// create a distinct overload from a method with the same ordinary parameter types.
struct NoReceiverModeOverload
{
    int read() { return 0; }
    /*CHECK:
        ^^^^ see previous definition of 'read'
    */

    int read() const { return 1; }
    /*CHECK:
        ^^^^ function 'read' already has a body
        ^^^^ redeclared here
    */
};
