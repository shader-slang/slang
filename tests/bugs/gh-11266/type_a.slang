module type_a;
import iface;

namespace m {

public struct ConcreteA : IFoo {
    public uint header;
    public uint payload_a;
    public int get_val() { return int(payload_a); }
}

}
