implementing reopen_ns;

namespace repro {

public extension<In1 : IBase, In2 : IBase> Add<In1, In2>
    : ICollector
    where In1 : ICollector
    where In2 : ICollector
{
    public int collect()
    {
        return this.in1.collect() + this.in2.collect();
    }
}

} // namespace repro
