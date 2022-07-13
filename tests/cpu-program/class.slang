//TEST:EXECUTABLE:
__target_intrinsic(cpp, "printf(\"%s\\n\", ($0).getBuffer())")
void writeln(String text);

class MyClass
{
    int intMember;
    __init()
    {
        intMember = 0;
    }
    int method()
    {
        writeln("method");
        return intMember;
    }
}

public __extern_cpp int main()
{
    MyClass obj = new MyClass();
    return obj.method();
}