// main.slang

__target_intrinsic(cpp, "printf(\"%s\", ($0).getBuffer())")
public void writeln(String text);

[DllImport("User32")]
int MessageBoxA(Ptr<void> hwnd, String text, String caption, uint flags);

[COM]
interface IObject
{
    int getValue(int value) throws int;
}

[DllImport("test-com")]
IObject createObject();

public __extern_cpp void main() throws int
{
    //writeln("hello world");
    //MessageBoxA(nullptr, "hello world!", "example", 0);

    IObject object = createObject();

    int rs = try object.getValue(2);
}