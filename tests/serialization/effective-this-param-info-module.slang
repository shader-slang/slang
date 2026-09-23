//TEST_IGNORE_FILE:

module "effective-this-param-info-module";

public struct Receiver
{
    public int value;

    [constref]
    public int read()
    {
        return value;
    }

    [__ref]
    public void increment()
    {
        value++;
    }
}
