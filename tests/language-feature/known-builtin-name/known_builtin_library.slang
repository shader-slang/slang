// All three spellings must produce the same integer-valued IR decoration.
enum BuiltinRole
{
    Append = 0,
}

[KnownBuiltin("GeometryStreamAppend")]
public uint fromName(uint value)
{
    return value + 1;
}

[KnownBuiltin(0)]
public uint fromInteger(uint value)
{
    return value + 2;
}

[KnownBuiltin(BuiltinRole.Append)]
public uint fromEnum(uint value)
{
    return value + 3;
}
