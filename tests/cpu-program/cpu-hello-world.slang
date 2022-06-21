//TEST:EXECUTABLE:
__target_intrinsic(cpp, "printf(\"%s\\n\", ($0).getBuffer())")
void writeln(String text);

public __extern_cpp int main()
{
    writeln("Hello World.");
    return 0;
}