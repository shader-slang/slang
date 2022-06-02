// shader.slang

// Example of using 'NativeString'

public __extern_cpp NativeString getString(NativeString in)
{
    return in;
}

[COM]
interface IDoThings
{
    int doThing(int a, int b);
    int calcHash(NativeString in);
}

public __extern_cpp int calcHash(NativeString text, IDoThings doThings)
{
    return doThings.calcHash(text);
}

