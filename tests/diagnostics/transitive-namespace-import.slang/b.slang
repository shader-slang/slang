module b;

namespace ns
{
    public int f_b() { return 2; }

    // A non-public (internal) function. It must never be offered as a
    // "did you mean" suggestion from another module, since lookup itself
    // would not resolve it.
    int f_private() { return 3; }
}
