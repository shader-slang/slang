
public interface IStorage<T>
{
    public associatedtype Address;
}

public interface IParameterStorage<T> : IStorage<T>
{
    // Crashes
    public void add(Address address, T value);

    // Crashes
    public Address getAddress(T address);

    // OK
    public void add(T address, T value);
}

public struct ParameterStorage<T> : IParameterStorage<T>
{
    public typealias Address = uint;

    public void add(Address address, T value)
    {
        
    }

    public Address getAddress(T address)
    {
        return Address(0);
    }

    public void add(T address, T value)
    {

    }
}

void computeMain()
{

}