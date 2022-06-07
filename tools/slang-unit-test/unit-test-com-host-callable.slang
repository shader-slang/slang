// shader.slang

// Example of using 'NativeString'

public __extern_cpp NativeString getString(NativeString in)
{
    return in;
}

public __extern_cpp __global int intGlobal = 10;

public __extern_cpp void setGlobal(int v)
{
    intGlobal = v;
}

public __extern_cpp int getGlobal()
{
    return intGlobal;
}

[COM]
interface IDoThings
{
    int doThing(int a, int b);
    int calcHash(NativeString in);
}

[COM]
interface ICountGood
{
    int nextCount();
}

public __extern_cpp __global ICountGood globalCounter;

public __extern_cpp void setCounter(ICountGood counter)
{
    globalCounter = counter;
}

public __extern_cpp int nextCount()
{
    return globalCounter.nextCount();
}

public __extern_cpp int calcHash(NativeString text, IDoThings doThings)
{
    return doThings.calcHash(text);
}
