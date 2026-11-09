from pathlib import Path

import slangvm

with slangvm.init() as vm:
    m = vm.load(Path(__file__).with_name("main.slang"))
    print(m.add(2, 3))
    print(m.scale(1.5, 2.0))

    producer = vm.load(Path(__file__).with_name("producer.slang"))
    point = producer.makePoint(3.0, 4.0)
    # Loading another program preserves the existing value's canonical type identity.
    consumer = vm.load(Path(__file__).with_name("consumer.slang"))
    print(point)
    print(consumer.lengthSquared(point))  # 25.0
    shifted = consumer.translate(point, 1.0, 2.0)
    print(consumer.lengthSquared(shifted))  # 52.0
    print(consumer.lengthSquared(point))  # 25.0

print(point)  # Cached type name remains readable; the native payload is closed.
