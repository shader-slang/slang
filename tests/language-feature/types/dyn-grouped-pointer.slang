//TEST:SIMPLE: -std legacy -no-codegen
//TEST:SIMPLE: -std 2025 -no-codegen
//TEST:SIMPLE: -std 2026 -no-codegen
//TEST:SIMPLE: -std 202c -no-codegen

// Suffixes after `(dyn IValue)` form the same pointer and array types as alias spellings.
// Returning each alias-based wrapper checks the type selected by expression parsing.
interface IValue
{
    int read();
}

struct Value : IValue
{
    int read() { return 7; }
}

struct Holder<T>
{
    T value;
}

struct Pair<T, U>
{
    T first;
    U second;
}

typealias Box = dyn IValue;
typealias BoxPointer = Box*;
typealias BoxPointerPointer = Box**;
typealias BoxPointerArray = Box*[2];
typealias BoxPointerPointerArray = Box**[2];

// A closing generic delimiter or comma leaves no operand for a dereference cast.
Holder<BoxPointer> pointerBeforeGenericClose(Holder<(dyn IValue)*> value)
{
    return value;
}

Pair<BoxPointer, int> pointerBeforeGenericComma(Pair<(dyn IValue)*, int> value)
{
    return value;
}

Holder<Holder<BoxPointer>> pointerBeforeNestedGenericClose(Holder<Holder<(dyn IValue)*>> value)
{
    return value;
}

// All stars belong to the type when the pointer sequence ends before a delimiter.
Holder<BoxPointerPointer> pointerSequence(Holder<(dyn IValue)**> value)
{
    return value;
}

// An array suffix can follow either one pointer suffix or a complete pointer sequence.
Holder<BoxPointerArray> pointerArray(Holder<(dyn IValue)*[2]> value)
{
    return value;
}

Holder<BoxPointerPointerArray> pointerSequenceArray(Holder<(dyn IValue)**[2]> value)
{
    return value;
}

// An identifier after the stars supplies a cast operand, so these remain dereference casts.
Box castDereferencedPointer(Value* value)
{
    return (dyn IValue)*value;
}

Box castTwiceDereferencedPointer(Value** value)
{
    return (dyn IValue)**value;
}

Box castThreeTimesDereferencedPointer(Value*** value)
{
    return (dyn IValue)***value;
}

// Parenthesized ordinary values still use multiplication rather than a type suffix or cast.
int multiplyParenthesizedValue(int left, int right)
{
    return (left)*right;
}
