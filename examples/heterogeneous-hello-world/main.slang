// main.slang

__target_intrinsic(cpp, "printf(\"%s\", ($0).getBuffer())")
public void writeln(String text);

[DllImport("User32")]
int MessageBoxA(Ptr<void> hwnd, String text, String caption, uint flags);

public __extern_cpp int main()
{
    writeln("hello world");
    MessageBoxA(nullptr, "hello world!", "example", 0);
    return 0;
}