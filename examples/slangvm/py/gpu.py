"""Pass a struct owning a GPU buffer from Slang to Python and into another Slang program."""

import argparse
import gc
from pathlib import Path

import slangvm


def main(backend="default"):
    with slangvm.init({"backend": backend, "working_directory": Path(__file__).parent}) as vm:
        producer = vm.load("gpu-producer.slang")
        original = producer.makeState(4, 10)
        consumer = vm.load("gpu-consumer.slang")
        copy = consumer.copyState(original)
        print(original)
        print("Initial GPU sum:", consumer.sum(original))  # 46
        consumer.add(copy, 1)
        print("GPU sum through the original:", consumer.sum(original))  # 50
        del original
        gc.collect()
        print("GPU sum after releasing the original:", consumer.sum(copy))  # 50

        buffer = producer.getBuffer(copy)
        del copy, producer
        gc.collect()
        print("Buffer after releasing both structs:", consumer.readValue(buffer, 0))  # 11
        # Python holds an opaque owning handle. All GPU access stays in the Slang helpers.
        print(buffer)

    # Close releases the native payload even while Python still holds the wrapper.
    print("After VM close:", buffer)
    del buffer


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--backend", default="default", help="RHI backend, e.g. vulkan or d3d12")
    main(parser.parse_args().backend)
