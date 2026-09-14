//TEST:SIMPLE: -no-codegen

uint acceptUInt(uint value)
{
    return value;
}

struct NotUInt
{};

NotUInt acceptUInt(int value)
{
    NotUInt result;
    return result;
}

static unsigned globalValue = 0;

unsigned int useUnsignedTypeNames(unsigned value)
{
    unsigned int localValue = (unsigned int)value;
    vector<unsigned int, 4> vectorValue = localValue;
    uint typeSize = sizeof(unsigned int);
    return acceptUInt(globalValue + vectorValue.x + typeSize);
}
