// main.slang

__target_intrinsic(cpp, "printf(\"%s\", ($0).getBuffer())")
public void writeln(String text);

public __extern_cpp int main()
{
    writeln("hello world");
    return 0;
}