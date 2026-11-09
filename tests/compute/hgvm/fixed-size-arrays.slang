//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

struct PaddedItem
{
    bool enabled;
    uint value;
}

struct ArrayHolder
{
    bool enabled;
    Array<uint, 4> values;
}

[noinline]
Array<uint, 4> makeValues(uint base)
{
    return __makeArray(base, base + 1, base + 2, base + 3);
}

[noinline]
uint readValue(Array<uint, 4> values, uint index)
{
    // A by-value array access reaches OpExtractElement instead of loading through an address.
    return values[index];
}

[noinline]
void writeValue(inout Array<uint, 4> values, uint index, uint value)
{
    // Mutating an array local reaches OpElementAddress and then uses an ordinary fixed-size store.
    values[index] = value;
}

[noinline]
PaddedItem makeItem(bool enabled, uint value)
{
    PaddedItem result;
    result.enabled = enabled;
    result.value = value;
    return result;
}

[noinline]
uint updateItem(inout Array<PaddedItem, 4> values, uint index, uint value)
{
    // PaddedItem has alignment four and size eight, so this checks that array element addressing
    // uses the precomputed eight-byte stride rather than a scalar-size assumption.
    values[index].value = value;
    return values[index].value;
}

[noinline]
uint readNested(Array<Array<uint, 4>, 4> values, uint row, uint column)
{
    return values[row][column];
}

[noinline]
uint readHolder(ArrayHolder value, uint index)
{
    return value.values[index];
}

[noinline]
uint updateHolder(inout ArrayHolder value, uint index, uint replacement)
{
    value.values[index] = replacement;
    return value.values[index];
}

[host]
void main()
{
    Array<uint, 4> values = makeValues(10);
    print(readValue(values, 2));
    writeValue(values, 1, 99);
    print(readValue(values, 1));

    Array<PaddedItem, 4> items = __makeArray(
        makeItem(false, 7),
        makeItem(true, 11),
        makeItem(false, 15),
        makeItem(true, 19));
    print(updateItem(items, 2, 42));

    Array<uint, 4> firstRow = __makeArray(1u, 2u, 5u, 6u);
    Array<uint, 4> secondRow = __makeArray(3u, 4u, 7u, 8u);
    Array<Array<uint, 4>, 4> grid =
        __makeArray(firstRow, secondRow, firstRow, secondRow);
    print(readNested(grid, 1, 0));

    ArrayHolder holder;
    holder.enabled = true;
    holder.values = makeValues(20);
    print(readHolder(holder, 2));
    print(updateHolder(holder, 3, 77));
}

// CHECK: 12
// CHECK-NEXT: 99
// CHECK-NEXT: 42
// CHECK-NEXT: 3
// CHECK-NEXT: 22
// CHECK-NEXT: 77
