// struct-generic-value-param-import.slang
//TEST_IGNORE_FILE:

// This file is used by `struct-generic-value-param.slang`,
// and also incidentally tests that a trailing `;` is optional
// for `struct` decalrations in Slang files, including
// any `import`ed code.

interface IData {}

struct Data<let kCount : int> : IData
{
    int state;

    [mutating] void doStuff()
    {
        state++;
    }
}
