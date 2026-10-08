implementing reopen_ns;

namespace repro {

public interface IBase
{
    int base_value();
}

public interface ICollector : IBase
{
    int collect();
}

public struct Add<In1 : IBase, In2 : IBase> : IBase
{
    public In1 in1;
    public In2 in2;

    public int base_value()
    {
        return in1.base_value() + in2.base_value();
    }
}

} // namespace repro
