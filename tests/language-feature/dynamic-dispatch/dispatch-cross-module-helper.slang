module "dispatch-cross-module-helper";

public interface ICrossModule
{
    int getValue();
}

public struct HelperImpl : ICrossModule
{
    int v;
    int getValue() { return v; }

    public __init(int val) { v = val; }
}
