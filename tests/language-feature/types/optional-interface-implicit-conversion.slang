//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-vk -shaderobj -output-using-type -Xslang -DEXPLICIT_CAST
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-vk -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutput
RWStructuredBuffer<int> gOutput;

interface ICash
{
    int getValue();
}

struct Cash : ICash
{
    int value;
    override int getValue() { return value; }
}

interface IBank
{
    associatedtype ConcreteCashType : ICash;

    Optional<ConcreteCashType> makeCash();
}

struct Bank : IBank
{
    typedef Cash ConcreteCashType;

    override Optional<ConcreteCashType> makeCash()
    {
        Cash cash;
        // CHK: 42
        cash.value = 42;
        return cash;
    }

    // Adding a dummy member variable, because an empty struct
    // may go through a different optimization pass
    int dummy;
    __init() { dummy = 0; }
}

Optional<ICash> makeCash<BankType : IBank>(BankType bank)
{
    Optional<BankType.ConcreteCashType> maybeCash = bank.makeCash();

#if defined(EXPLICIT_CAST)
    if (let raw = maybeCash)
        return Optional<ICash>(raw);
    return none;
#else
    // Implicit conversion:
    // Optional<Cash> -> Optional<ICash>
    return maybeCash;
#endif
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Bank bank;

    Optional<ICash> maybeCash = makeCash(bank);

    if (let v = maybeCash)
    {
        gOutput[0] = v.getValue();
    }
}

