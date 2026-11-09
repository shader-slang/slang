"""End-to-end tests against the matching, locally built slangvm extension."""

import gc
import math
import os
from pathlib import Path
import struct
import tempfile
import unittest

import slangvm

EXAMPLES = Path(__file__).resolve().parents[2] / "examples" / "slangvm" / "py"


SOURCE = """
[host] int add(int a, int b) { return a + b; }
[host] uint unsignedIdentity(uint x) { return x; }
[host] float scale(float x, float factor) { return x * factor; }
[host] float floatIdentity(float x) { return x; }
[host] half halfIdentity(half x) { return x; }
[host] double doubleIdentity(double x) { return x; }
[host] bool invert(bool x) { return !x; }
[host] void consume(int x) { }
[host] int divide(int x) { return 24 / x; }
[host] float2 vectorIdentity(float2 x) { return x; }
[host] float2 vectorResult() { return float2(1, 2); }
[host] void mutate(inout int x) { x += 1; }
[noinline] int inner(int x) { return x * 3; }
[host] int nested(int x) { return inner(inner(x)); }
// Loading a module must not invoke main.
[host] void main()
{
    String message = "main must not run during load";
    hgbc_asm { OpFail $message; };
}
"""


class ScalarInterop(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        workspace = Path(__file__).resolve().parents[2] / "build" / "slangvm-tests"
        workspace.mkdir(parents=True, exist_ok=True)
        cls.temp = tempfile.TemporaryDirectory(dir=workspace)
        cls.directory = Path(cls.temp.name)
        (cls.directory / "main.slang").write_text(SOURCE)

    @classmethod
    def tearDownClass(cls):
        gc.collect()
        cls.temp.cleanup()

    def setUp(self):
        self.vm = slangvm.init({"working_directory": self.directory})
        self.m = self.vm.load("main.slang")

    def tearDown(self):
        self.vm.close()

    def test_backend_settings(self):
        with slangvm.init({"backend": "none", "working_directory": EXAMPLES}) as vm:
            self.assertEqual(vm.load("main.slang").add(2, 3), 5)
        with self.assertRaisesRegex(ValueError, "backend"):
            slangvm.init({"backend": "invalid-backend"})
        with self.assertRaises(TypeError):
            slangvm.init({"backend": 123})
        with self.assertRaisesRegex(TypeError, "Unknown slangvm settings"):
            slangvm.init({"back_end": "vulkan"})

    def test_gpu_requires_explicit_backend(self):
        with slangvm.init({"working_directory": EXAMPLES}) as vm:
            with self.assertRaisesRegex(RuntimeError, "requires an RHI device"):
                vm.load("gpu-producer.slang").makeState(4, 10)
            self.assertEqual(vm.load("main.slang").add(2, 3), 5)

    def test_bundled_standard_modules(self):
        # The working directory contains only this consumer, with no library search paths.
        # On Windows this also exercises the compiler DLL copied into the Python package.
        source = """
import slang.runtime;
import slang.tensor.differentiable;
import slang.compute.buffer_utils;
import slang.compute.texture_utils;
[host] uint gridSize() { return Grid(7u).size; }
"""
        (self.directory / "standard-modules.slang").write_text(source, encoding="utf-8")
        self.assertEqual(self.vm.load("standard-modules.slang").gridSize(), 7)

    def test_scalar_calls_and_discovery(self):
        self.assertEqual(self.m.add(2, 3), 5)
        self.assertEqual(self.m.add(-(2**31), 0), -(2**31))
        self.assertEqual(self.m.add(2**31 - 1, 0), 2**31 - 1)
        self.assertEqual(self.m.unsignedIdentity(2**32 - 1), 2**32 - 1)
        self.assertEqual(self.m.scale(1.5, 2), 3.0)
        self.assertIs(self.m.invert(False), True)
        self.assertIs(self.m.invert(True), False)
        self.assertIsNone(self.m.consume(3))
        self.assertIs(self.m.add, self.m["add"])
        self.assertIn("add", dir(self.m))
        with self.assertRaises(AttributeError):
            self.m.missing
        for i in range(100):
            self.assertEqual(self.m.nested(i), i * 9)

    def test_precision_and_special_values(self):
        x = 1.2345678901234567
        self.assertEqual(self.m.doubleIdentity(x), x)
        self.assertEqual(self.m.floatIdentity(x), struct.unpack("f", struct.pack("f", x))[0])
        self.assertEqual(self.m.halfIdentity(x), struct.unpack("e", struct.pack("e", x))[0])
        for f in (self.m.halfIdentity, self.m.floatIdentity, self.m.doubleIdentity):
            self.assertTrue(math.isnan(f(float("nan"))))
            self.assertEqual(f(float("inf")), float("inf"))
            self.assertEqual(math.copysign(1.0, f(-0.0)), -1.0)

    def test_rejects_bad_arguments(self):
        for x in (-(2**31) - 1, 2**31, 2**100):
            with self.assertRaises(OverflowError):
                self.m.add(x, 0)
        for x in (-1, 2**32):
            with self.assertRaises(OverflowError):
                self.m.unsignedIdentity(x)
        for x in (True, 1.0, "1", None):
            with self.assertRaises(TypeError):
                self.m.add(x, 0)
        with self.assertRaises(TypeError):
            self.m.invert(1)
        with self.assertRaises(TypeError):
            self.m.scale(True, 1)
        with self.assertRaises(TypeError):
            self.m.add(1)
        with self.assertRaises(TypeError):
            self.m.add(a=1, b=2)
        with self.assertRaises(TypeError):
            self.m.vectorIdentity(1)
        self.assertIsInstance(self.m.vectorResult(), slangvm.Value)
        with self.assertRaises(TypeError):
            self.m.mutate(1)
        with self.assertRaises(OverflowError):
            self.m.floatIdentity(1e300)
        with self.assertRaises(OverflowError):
            self.m.halfIdentity(70000)
        self.assertEqual(self.m.add(1, 2), 3)

    def test_opaque_vector_roundtrip(self):
        value = self.m.vectorResult()
        self.assertIsInstance(self.m.vectorIdentity(value), slangvm.Value)
        self.assertTrue(value.type_name)
        self.assertIn("slangvm.Value", repr(value))
        with self.assertRaises(TypeError):
            self.m.add(value, 1)

    def test_runtime_failure_and_recovery(self):
        with self.assertRaisesRegex(RuntimeError, "division by zero"):
            self.m.divide(0)
        self.assertEqual(self.m.divide(3), 8)

    def test_lifetime_and_independent_vms(self):
        other = slangvm.init({"working_directory": self.directory})
        function = other.load("main.slang").add
        del other
        gc.collect()
        self.vm.close()
        self.assertEqual(function(1, 2), 3)
        with self.assertRaisesRegex(RuntimeError, "closed"):
            self.m.add(1, 2)
        with self.assertRaisesRegex(RuntimeError, "closed"):
            self.vm.load("main.slang")
        self.vm.close()

    def test_exact_paths_and_imports(self):
        first = self.directory / "first"
        second = self.directory / "second"
        first.mkdir(exist_ok=True)
        second.mkdir(exist_ok=True)
        (first / "main.slang").write_text("[host] int answer() { return 1; }")
        (second / "main.slang").write_text("[host] int answer() { return 2; }")
        self.assertEqual(self.vm.load(first / "main.slang").answer(), 1)
        self.assertEqual(self.vm.load(second / "main.slang").answer(), 2)
        (first / "helper.slang").write_text("public int helper() { return 17; }")
        (first / "importer.slang").write_text("import helper; [host] int answer() { return helper(); }")
        self.assertEqual(self.vm.load(first / "importer.slang").answer(), 17)

    def test_load_errors(self):
        with self.assertRaises(RuntimeError):
            self.vm.load("missing.slang")
        (self.directory / "invalid.slang").write_text("[host] int bad() { return missingName; }")
        with self.assertRaisesRegex(RuntimeError, "missingName"):
            self.vm.load("invalid.slang")
        (self.directory / "empty.slang").write_text("int helper() { return 1; }")
        with self.assertRaisesRegex(RuntimeError, "no .*host"):
            self.vm.load("empty.slang")


GEOMETRY = """
public struct Point { public float x; public float y; }
public struct Composite
{
    public bool enabled;
    public Point origin;
    public float3 directions[8];
    public float2x2 basis;
}
public struct Empty { }
"""

PRODUCER = """
import geometry;
[host] Point makePoint(float x, float y) { return Point(x, y); }
[host] Composite makeComposite()
{
    Composite c;
    c.enabled = true;
    c.origin = Point(3, 4);
    for (int i = 0; i < 8; ++i) c.directions[i] = float3(i, i + 1, i + 2);
    c.basis = float2x2(1, 2, 3, 4);
    return c;
}
"""

CONSUMER = """
import geometry;
[host] float lengthSquared(Point p) { return p.x * p.x + p.y * p.y; }
[host] Point translate(Point p, float dx, float dy) { return Point(p.x + dx, p.y + dy); }
[host] float inspectComposite(Composite c)
{
    return c.origin.x + c.origin.y + c.directions[7].z + c.basis[1][0] +
           (c.enabled ? 1 : 0);
}
[host] Composite copyComposite(Composite c) { return c; }
[host] int consumeEmpty(Empty value) { return 42; }
[host] Point failingTransform(Point p, int divisor)
{
    int factor = 24 / divisor;
    return Point(p.x + factor, p.y);
}
"""


class ValueInterop(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        workspace = Path(__file__).resolve().parents[2] / "build" / "slangvm-tests"
        workspace.mkdir(parents=True, exist_ok=True)
        cls.temp = tempfile.TemporaryDirectory(dir=workspace)
        cls.directory = Path(cls.temp.name)
        for name, source in (
            ("geometry", GEOMETRY), ("producer", PRODUCER), ("consumer", CONSUMER)
        ):
            (cls.directory / f"{name}.slang").write_text(source)
        (cls.directory / "empty_producer.slang").write_text("""
            import geometry;
            [host] Empty makeEmpty() { return Empty(); }
        """)
        (cls.directory / "unrelated.slang").write_text("""
            struct Point { float x; float y; }
            [host] Point makePoint() { return Point(3, 4); }
        """)
        (cls.directory / "unsupported.slang").write_text("""
            [host] void change(inout int x) { x += 1; }
        """)

    @classmethod
    def tearDownClass(cls):
        gc.collect()
        cls.temp.cleanup()

    def setUp(self):
        self.vm = slangvm.init({"working_directory": self.directory})

    def tearDown(self):
        self.vm.close()

    def test_cross_program_values_and_loading(self):
        producer = self.vm.load("producer.slang")
        p = producer.makePoint(3, 4)
        self.assertIsInstance(p, slangvm.Value)
        self.assertIn("Point", p.type_name)
        consumer = self.vm.load("consumer.slang")
        self.assertEqual(consumer.lengthSquared(p), 25)
        q = consumer.translate(p, 1, 2)
        self.assertEqual(consumer.lengthSquared(q), 52)
        self.assertEqual(consumer.lengthSquared(p), 25)
        del producer
        gc.collect()
        self.assertEqual(consumer.lengthSquared(p), 25)
        for _ in range(20):
            self.assertEqual(consumer.lengthSquared(consumer.translate(p, 0, 0)), 25)

    def test_reverse_load_order_large_nested(self):
        consumer = self.vm.load("consumer.slang")
        producer = self.vm.load("producer.slang")
        value = producer.makeComposite()
        self.assertEqual(consumer.inspectComposite(value), 20)
        self.assertEqual(consumer.inspectComposite(consumer.copyComposite(value)), 20)
        self.assertEqual(consumer.lengthSquared(producer.makePoint(3, 4)), 25)

    def test_empty_struct(self):
        consumer = self.vm.load("consumer.slang")
        producer = self.vm.load("empty_producer.slang")
        self.assertEqual(consumer.consumeEmpty(producer.makeEmpty()), 42)

    def test_nominal_types_and_vm_ownership(self):
        producer = self.vm.load("producer.slang")
        consumer = self.vm.load("consumer.slang")
        with self.assertRaises(TypeError):
            consumer.lengthSquared(self.vm.load("unrelated.slang").makePoint())
        with slangvm.init({"working_directory": self.directory}) as other:
            foreign = other.load("producer.slang").makePoint(3, 4)
            with self.assertRaises(TypeError):
                consumer.lengthSquared(foreign)
        with self.assertRaises(TypeError):
            consumer.lengthSquared(1)
        with self.assertRaises(TypeError):
            consumer.lengthSquared(producer.makeComposite())
        self.assertEqual(consumer.lengthSquared(producer.makePoint(3, 4)), 25)

    def test_close_and_runtime_failure(self):
        producer = self.vm.load("producer.slang")
        consumer = self.vm.load("consumer.slang")
        p = producer.makePoint(3, 4)
        with self.assertRaisesRegex(RuntimeError, "division by zero"):
            consumer.failingTransform(p, 0)
        self.assertEqual(consumer.lengthSquared(p), 25)
        bound = consumer.lengthSquared
        name = p.type_name
        self.vm.close()
        self.vm.close()
        self.assertEqual(p.type_name, name)
        self.assertIn("closed", repr(p))
        self.assertIn("lengthSquared", dir(consumer))
        with self.assertRaisesRegex(RuntimeError, "closed"):
            bound(p)
        with self.assertRaisesRegex(RuntimeError, "closed"):
            consumer.translate(p, 1, 2)

    def test_unsupported_reference_signatures(self):
        module = self.vm.load("unsupported.slang")
        with self.assertRaises(TypeError):
            module.change(1)


MANAGED_TYPES = """
public class Counter
{
    public int value;
    public __init(int initialValue) { value = initialValue; }
}
public struct CounterPair { public Counter first; public Counter second; }
public struct State
{
    public CounterPair pair;
    public RuntimeArray<Counter> counters;
    public String label;
}
"""

MANAGED_PRODUCER = """
import managed_types;
[host] State makeState(int value)
{
    Counter counter = new Counter(value);
    RuntimeArray<Counter> counters = RuntimeArray<Counter>::alloc(2);
    counters[0] = counter;
    counters[1] = counter;
    String pattern = "state {}";
    String label = hgbc_asm { result : $$String = OpFormatString $pattern $$int $value; };
    return State(CounterPair(counter, counter), counters, label);
}
[host] Counter getCounter(State state) { return state.pair.first; }
[host] RuntimeArray<Counter> getCounters(State state) { return state.counters; }
[host] String getLabel(State state) { return state.label; }
[host] String makeLiteral() { return "literal text"; }
[host] State makeThenFail(int value, int divisor)
{
    State state = makeState(value);
    state.pair.first.value = 24 / divisor;
    return state;
}
"""

MANAGED_CONSUMER = """
import managed_types;
[host] State copyState(State state) { return state; }
[host] void advance(State state, int amount) { state.pair.first.value += amount; }
[host] int readState(State state)
{
    return state.pair.first.value + state.pair.second.value +
        state.counters[0].value + state.counters[1].value;
}
[host] int readCounter(Counter counter) { return counter.value; }
[host] void advanceCounter(Counter counter, int amount) { counter.value += amount; }
[host] int readArray(RuntimeArray<Counter> counters)
{
    return counters[0].value + counters[1].value;
}
[host] void advanceArray(RuntimeArray<Counter> counters, int amount)
{
    counters[1].value += amount;
}
[host] uint textBytes(String text)
{
    return hgbc_asm { result : $$uint = OpStringGetByteCount $text; };
}
[host] String copyText(String text) { return text; }
"""


class ManagedValueInterop(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        workspace = Path(__file__).resolve().parents[2] / "build" / "slangvm-tests"
        workspace.mkdir(parents=True, exist_ok=True)
        cls.temp = tempfile.TemporaryDirectory(dir=workspace)
        cls.directory = Path(cls.temp.name)
        for name, source in (
            ("managed_types", MANAGED_TYPES),
            ("managed_producer", MANAGED_PRODUCER),
            ("managed_consumer", MANAGED_CONSUMER),
        ):
            (cls.directory / f"{name}.slang").write_text(source)
        declarations = """
            __target_intrinsic(hgbc, "OpTimerType") struct Timer {}
            __target_intrinsic(hgbc, "OpWindowType") struct Window {}
            __target_intrinsic(hgbc, "OpBoxType") struct Box {}
            struct Holder<T> { T first; T second; }
        """
        producer = "import owning_types;\n"
        consumer = "import owning_types;\n"
        cls.owning_types = ("Timer", "RWTexture2D<float4>",
                            "RaytracingAccelerationStructure", "Window", "Box",
                            "DescriptorHandle<RWTexture2D<float4>>", "Defer<uint>")
        for i, value_type in enumerate(cls.owning_types):
            operation = "OpCreateTimer" if value_type == "Timer" else "OpNullLit"
            producer += f"""
                [host] {value_type} make{i}() {{
                    return hgbc_asm {{ result : $${value_type} = {operation}; }};
                }}
                [host] Holder<{value_type}> wrap{i}({value_type} value) {{
                    return Holder<{value_type}>(value, value);
                }}
            """
            consumer += f"""
                [host] {value_type} copy{i}({value_type} value) {{ return value; }}
                [host] {value_type} unwrap{i}(Holder<{value_type}> value) {{
                    return value.second;
                }}
            """
        for name, source in (("owning_types", declarations), ("owning_producer", producer),
                             ("owning_consumer", consumer)):
            (cls.directory / f"{name}.slang").write_text(source)

    @classmethod
    def tearDownClass(cls):
        gc.collect()
        cls.temp.cleanup()

    def setUp(self):
        self.vm = slangvm.init({"working_directory": self.directory})

    def tearDown(self):
        self.vm.close()

    def test_resource_handle_roundtrips(self):
        producer = self.vm.load("owning_producer.slang")
        consumer = self.vm.load("owning_consumer.slang")
        survivors = []
        for i, value_type in enumerate(self.owning_types):
            with self.subTest(type=value_type):
                original = getattr(producer, f"make{i}")()
                self.assertIsInstance(original, slangvm.Value)
                nested = getattr(producer, f"wrap{i}")(original)
                alias = getattr(consumer, f"copy{i}")(original)
                del original
                gc.collect()
                unpacked = getattr(consumer, f"unwrap{i}")(nested)
                self.assertEqual(alias.type_name, unpacked.type_name)
                survivors.append(unpacked)
        self.vm.close()
        for value in survivors:
            self.assertIn("closed", repr(value))

    def test_cross_program_aliases_and_finalization(self):
        producer = self.vm.load("managed_producer.slang")
        value = producer.makeState(7)
        consumer = self.vm.load("managed_consumer.slang")
        copy = consumer.copyState(value)
        self.assertIsInstance(value, slangvm.Value)
        self.assertIn("State", value.type_name)
        consumer.advance(copy, 3)
        self.assertEqual(consumer.readState(value), 40)
        counter = producer.getCounter(value)
        counters = producer.getCounters(value)
        label = producer.getLabel(value)
        literal = producer.makeLiteral()
        del value, copy, producer
        gc.collect()
        consumer.advanceCounter(counter, 2)
        self.assertEqual(consumer.readArray(counters), 24)
        del counter
        gc.collect()
        consumer.advanceArray(counters, 1)
        self.assertEqual(consumer.readArray(counters), 26)
        label_copy = consumer.copyText(label)
        del label
        gc.collect()
        self.assertEqual(consumer.textBytes(label_copy), 7)
        self.assertEqual(consumer.textBytes(literal), 12)

    def test_reverse_load_order_failure_and_type_checks(self):
        consumer = self.vm.load("managed_consumer.slang")
        producer = self.vm.load("managed_producer.slang")
        value = producer.makeState(5)
        for _ in range(5):
            with self.assertRaisesRegex(RuntimeError, "division by zero"):
                producer.makeThenFail(99, 0)
            self.assertEqual(consumer.readState(value), 20)
        with self.assertRaises(TypeError):
            consumer.readState(producer.getCounter(value))
        with slangvm.init({"working_directory": self.directory}) as other:
            foreign = other.load("managed_producer.slang").makeState(5)
            with self.assertRaises(TypeError):
                consumer.readState(foreign)
        self.assertEqual(consumer.readState(consumer.copyState(value)), 20)

    def test_close_with_surviving_managed_values(self):
        producer = self.vm.load("managed_producer.slang")
        consumer = self.vm.load("managed_consumer.slang")
        value = producer.makeState(7)
        counter = producer.getCounter(value)
        counters = producer.getCounters(value)
        label = producer.getLabel(value)
        names = [item.type_name for item in (value, counter, counters, label)]
        self.vm.close()
        self.vm.close()
        self.assertEqual([item.type_name for item in (value, counter, counters, label)], names)
        for item in (value, counter, counters, label):
            self.assertIn("closed", repr(item))
        with self.assertRaisesRegex(RuntimeError, "closed"):
            consumer.readState(value)
        del value, counter, counters, label
        gc.collect()


class GenericValueInterop(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        workspace = Path(__file__).resolve().parents[2] / "build" / "slangvm-tests"
        workspace.mkdir(parents=True, exist_ok=True)
        cls.temp = tempfile.TemporaryDirectory(dir=workspace)
        cls.directory = Path(cls.temp.name)
        sources = {
            "generic_types": """
                public enum First : int { zero = 0 }
                public enum Second : int { zero = 0 }
                public struct Token<T> { public T value; }
                public struct Wrapper<T> { public T value; }
                public struct Outer<T>
                {
                    public struct Inner<U> { public T x; public U y; }
                }
            """,
            "alpha": "public struct Item { public float value; }",
            "beta": "public struct Item { public float value; }",
            "generic_producer": """
                import generic_types;
                import alpha;
                Wrapper<T> wrap<T>(T value) { return Wrapper<T>(value); }
                [host] Token<First> makeToken(int value)
                {
                    return Token<First>(First(value));
                }
                [host] Wrapper<Wrapper<Item[2]>> makeNested(float value)
                {
                    Item items[2] = { Item(value), Item(value + 1) };
                    return wrap(wrap(items));
                }
                [host] Outer<float>.Inner<int> makeInner(float x, int y)
                {
                    return Outer<float>.Inner<int>(x, y);
                }
            """,
            "generic_consumer": """
                import generic_types;
                import alpha;
                [host] int readToken(Token<Second> value) { return int(value.value); }
                [host] float readNested(Wrapper<Wrapper<Item[2]>> value)
                {
                    return value.value.value[0].value + value.value.value[1].value;
                }
                [host] float readInner(Outer<float>.Inner<int> value)
                {
                    return value.x + float(value.y);
                }
            """,
            "generic_unrelated": """
                import generic_types;
                import beta;
                [host] float readNested(Wrapper<Wrapper<Item[2]>> value)
                {
                    return value.value.value[0].value + value.value.value[1].value;
                }
            """,
        }
        for name, source in sources.items():
            (cls.directory / f"{name}.slang").write_text(source)

    @classmethod
    def tearDownClass(cls):
        gc.collect()
        cls.temp.cleanup()

    def test_lowered_generic_identity_across_programs(self):
        for consumer_first in (False, True):
            with self.subTest(consumer_first=consumer_first):
                with slangvm.init({"working_directory": self.directory}) as vm:
                    if consumer_first:
                        consumer = vm.load("generic_consumer.slang")
                    producer = vm.load("generic_producer.slang")
                    nested = producer.makeNested(3)
                    if not consumer_first:
                        consumer = vm.load("generic_consumer.slang")
                    self.assertEqual(consumer.readNested(nested), 7)
                    self.assertEqual(consumer.readInner(producer.makeInner(1.5, 2)), 3.5)
                    # First and Second both erase to int before generic specialization.
                    self.assertEqual(consumer.readToken(producer.makeToken(17)), 17)

    def test_nested_nominal_arguments_remain_distinct(self):
        with slangvm.init({"working_directory": self.directory}) as vm:
            producer = vm.load("generic_producer.slang")
            nested = producer.makeNested(3)
            unrelated = vm.load("generic_unrelated.slang")
            # alpha.Item and beta.Item have identical fields and display names, including
            # inside Array and Wrapper, but they are different surviving nominal types.
            with self.assertRaises(TypeError):
                unrelated.readNested(nested)
            self.assertEqual(vm.load("generic_consumer.slang").readNested(nested), 7)


@unittest.skipUnless(os.environ.get("SLANGVM_TEST_BACKEND"), "Set SLANGVM_TEST_BACKEND to run GPU tests")
class GPUValueInterop(unittest.TestCase):
    def open_vm(self):
        return slangvm.init({
            "backend": os.environ["SLANGVM_TEST_BACKEND"],
            "working_directory": EXAMPLES,
        })

    def test_cross_program_gpu_values_and_collection(self):
        with self.open_vm() as vm:
            producer = vm.load("gpu-producer.slang")
            original = producer.makeState(4, 10)
            consumer = vm.load("gpu-consumer.slang")
            copy = consumer.copyState(original)
            self.assertIsInstance(original, slangvm.Value)
            self.assertIn("GpuState", original.type_name)
            self.assertEqual(consumer.sum(original), 46)
            consumer.add(copy, 1)
            self.assertEqual(consumer.sum(original), 50)
            del original
            gc.collect()
            self.assertEqual(consumer.sum(copy), 50)
            buffer = producer.getBuffer(copy)
            del copy, producer
            gc.collect()
            self.assertEqual(consumer.readValue(buffer, 0), 11)
            with self.assertRaisesRegex(RuntimeError, "out of range"):
                consumer.readValue(buffer, 4)
            self.assertEqual(consumer.readValue(buffer, 3), 14)

    def test_gpu_value_vm_and_type_checks(self):
        with self.open_vm() as vm, self.open_vm() as other:
            # Reverse load order must preserve the same canonical resource/struct types.
            consumer = vm.load("gpu-consumer.slang")
            producer = vm.load("gpu-producer.slang")
            value = producer.makeState(4, 10)
            buffer = producer.getBuffer(value)
            with self.assertRaises(TypeError):
                consumer.sum(buffer)
            foreign = other.load("gpu-producer.slang").makeState(4, 1)
            with self.assertRaisesRegex(TypeError, "another.*VM"):
                consumer.sum(foreign)
            self.assertEqual(consumer.sum(value), 46)

    def test_close_with_outstanding_gpu_values_and_work(self):
        vm = self.open_vm()
        try:
            producer = vm.load("gpu-producer.slang")
            consumer = vm.load("gpu-consumer.slang")
            value = producer.makeState(256, 1)
            buffer = producer.getBuffer(value)
            consumer.add(value, 1)
            # There has been no readback: close must drain any outstanding GPU submission.
            vm.close()
            vm.close()
            for item in (value, buffer):
                self.assertIn("closed", repr(item))
                self.assertTrue(item.type_name)
            with self.assertRaisesRegex(RuntimeError, "closed"):
                consumer.sum(value)
            del value, buffer
            gc.collect()
        finally:
            vm.close()

    def test_implicit_teardown_with_surviving_gpu_value(self):
        vm = self.open_vm()
        producer = vm.load("gpu-producer.slang")
        value = producer.makeState(256, 1)
        del producer, vm
        gc.collect()
        self.assertIn("closed", repr(value))
        del value
        gc.collect()


if __name__ == "__main__":
    unittest.main(verbosity=2)
