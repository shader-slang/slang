//TEST:EXECUTABLE:
__target_intrinsic(cpp, "printf(\"%s\\n\", ($0).getBuffer())")
void writeln(String text);

public __extern_cpp int main()
{
    uint2 value;
    int *pValue = (int*)&value;
    *pValue = 1;
    (*pValue)++;
    ++pValue[0];
    ++pValue;
    *pValue = 1;
    pValue = (int *)&value;
    int64_t ptrVal = int64_t(pValue);
    pValue = (int *)ptrVal;
    if (pValue
        && pValue != nullptr
        && ptrVal != 0
        && value[0] == 3
        && pValue[1] == 1)
        writeln("Success");
    else
        writeln("Fail");
    return 0;
}