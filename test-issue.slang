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

    // OK
    public void add(uint address, T value);
}