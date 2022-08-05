//TEST:EXECUTABLE:
__target_intrinsic(cpp, "printf(\"%s\\n\", ($0).getBuffer())")
void writeln(String text);

struct SubRecord
{
    int field2;
    float field3;
}

struct Record
{
    int field;
    SubRecord sub;
}

public __extern_cpp int main()
{
    Record rec;
    Record *pRec = &rec;
    pRec.field = 1;
    pRec.sub.field2 = 2;
    pRec.sub.field3 = 3.0f;
    if (rec.field == 1 && rec.sub.field2 == 2 && pRec.sub.field3 == 3.0f)
    {
        writeln("success");
    }
    else
    {
        writeln("fail");
    }
    return 0;
}