//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main

// Test using associated types from inherited interfaces

public interface IStorage<T>
{
    public associatedtype Address;
}

public interface IParameterStorage<T> : IStorage<T>
{
    // Should not crash - using associated type from parent interface
    public void add(Address address, T value);

    // Should not crash - returning associated type from parent interface
    public Address getAddress(T address);
    
    // Should still work - using concrete types
    public void add(T address, T value);

    // Should still work - using built-in types
    public void add(uint address, T value);
}

void main()
{
    // Simple test to ensure compilation succeeds
    // CHECK: void main
    return;
}