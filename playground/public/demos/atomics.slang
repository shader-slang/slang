import playground;

groupshared Atomic<int> atomicVariable;

void printMain()
{
    int replacedValue;

    print("store %d", 5);
    atomicVariable.store(5);
    print(": value is now %d\n", atomicVariable.load());

    print("exchange with %d", 7);
    replacedValue = atomicVariable.exchange(7);
    print(": value was %d, and is now %d\n", replacedValue, atomicVariable.load());

    print("exchange with %d if %d matches the stored value", 9, 42);
    replacedValue = atomicVariable.compareExchange(42, 9);
    print(": value was %d, and is now %d\n", replacedValue, atomicVariable.load());

    print("exchange with %d if %d matches the stored value", 9, 7);
    replacedValue = atomicVariable.compareExchange(7, 9);
    print(": value was %d, and is now %d\n", replacedValue, atomicVariable.load());

    print("replace with minimum of stored value and %d", 13);
    atomicVariable.min(13);
    print(": value is now %d\n", atomicVariable.load());

    print("replace with minimum of stored value and %d", 4);
    atomicVariable.min(4);
    print(": value is now %d\n", atomicVariable.load());

    print("replace with maximum of stored value and %d", 3);
    atomicVariable.max(3);
    print(": value is now %d\n", atomicVariable.load());

    print("replace with maximum of stored value and %d", 9);
    atomicVariable.max(9);
    print(": value is now %d\n", atomicVariable.load());

    print("replace with bitwise XOR of stored value and %d", 15);
    atomicVariable.xor(15);
    print(": value is now %d\n", atomicVariable.load());

    print("replace with bitwise OR of stored value and %d", 3);
    atomicVariable.or(3);
    print(": value is now %d\n", atomicVariable.load());

    print("subtract %d from the stored value", 5);
    atomicVariable.sub(5);
    print(": value is now %d\n", atomicVariable.load());

    print("add %d to the stored value", 3);
    atomicVariable.add(3);
    print(": value is now %d\n", atomicVariable.load());

    print("decrement");
    atomicVariable.decrement();
    print(": value is now %d\n", atomicVariable.load());

    print("increment");
    atomicVariable.increment();
    print(": value is now %d\n", atomicVariable.load());
}