//TEST:INTERPRET(hgvm):

void fail(String message)
{
    hgbc_asm
    {
        OpFail $message;
    };
}

void check(bool condition, String message)
{
    if (!condition)
        fail(message);
}

struct Pair
{
    uint x;
    uint y;
}

struct Holder
{
    Array<Pair, 2> pairs;
}

Array<uint, 4> makeValues(uint base)
{
    return __makeArray(base, base + 1, base + 2, base + 3);
}

uint readValue(Array<uint, 4> values, uint index)
{
    return values[index];
}

[host]
void main()
{
    Array<uint, 4> values = makeValues(10);
    check(readValue(values, 3) == 13, "fixed array call or return failed");

    uint index = 2;
    values[index] = 99;
    check(values[2] == 99, "fixed array indexed storage failed");

    Pair first;
    first.x = 1;
    first.y = 2;
    Pair second;
    second.x = 3;
    second.y = 4;

    Holder holder;
    holder.pairs[0] = first;
    holder.pairs[1] = second;
    Array<Pair, 2> copy = holder.pairs;
    check(copy[1].y == 4, "array-valued struct field failed");

    copy[0].x = 17;
    check(copy[0].x == 17, "struct-valued array element failed");
}
