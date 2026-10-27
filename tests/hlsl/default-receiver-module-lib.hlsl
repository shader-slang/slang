// Non-test helper module for default-receiver-imported-module.slang (no //TEST: directive).
// The consuming test precompiles it to a `.slang-module`, so that its methods' receiver
// information reaches the consumer through AST serialization.

interface IGetter
{
    int get();
};

struct Helper : IGetter
{
    int value;

    int get() { return value; }
};
