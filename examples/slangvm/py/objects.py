"""Return a struct and a class reference from Slang, with no GPU required."""

import gc
from pathlib import Path

import slangvm


def main():
    with slangvm.init({"working_directory": Path(__file__).parent}) as vm:
        m = vm.load("objects.slang")
        counters = m.makeCounters(7)
        counter = m.getFirst(counters)
        print(counters, counter)
        m.add(counter, 3)
        print("Sum through two aliases:", m.sum(counters))  # 20
        del counters
        gc.collect()
        # The returned Counter owns its reference even after the enclosing struct disappears.
        m.add(counter, 2)
        print("Counter after releasing the struct:", m.read(counter))  # 12

    print("After VM close:", counter)  # Cached type name is safe; the value is invalidated.
    del counter


if __name__ == "__main__":
    main()
