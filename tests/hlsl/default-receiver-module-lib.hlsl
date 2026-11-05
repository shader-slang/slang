// Non-test helper module for default-receiver-imported-module.slang (no //TEST: directive).
// The consuming Slang test precompiles it to a `.slang-module` and calls its methods.

interface IGetter
{
    int get();
};

struct Helper : IGetter
{
    int value;

    int get() { return value; }
};
