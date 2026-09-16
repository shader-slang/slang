//TEST:INTERPRET(filecheck=CHECK):

// Runtime output cannot distinguish a by-value (`In`) `this` from an
// immutable-borrow (`[constref]` -> `BorrowIn`) `this`, so this test pins only
// the observable axis: whether a write through `this` reaches the caller.

struct Counter
{
    int value;

    // Plain method: `this` is passed by value; a read-only view of the field.
    int read() { return value; }

    // `[mutating]`: `this` is a mutable borrow, so the write is visible to the caller.
    [mutating]
    void bump() { value = value + 1; }

    // `[constref]`: `this` is an immutable borrow; only reads are allowed.
    [constref]
    int doubled() { return value * 2; }

    property int prop
    {
        get { return value; }
        // A `set` accessor mutates through `this`.
        set(newValue) { value = newValue; }
    }
}

void main()
{
    Counter c = {10};

    //CHECK: read=10
    printf("read=%d\n", c.read());

    c.bump();
    //CHECK: afterBump=11
    printf("afterBump=%d\n", c.value);

    //CHECK: doubled=22
    printf("doubled=%d\n", c.doubled());

    c.prop = 7;
    //CHECK: prop=7
    printf("prop=%d\n", c.prop);
}
