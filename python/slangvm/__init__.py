"""Python scalars and opaque values for Slang's host-graph virtual machine."""

from pathlib import Path

from . import _slangvm

Value = _slangvm.Value


class Module:
    """An immutable compiled module whose functions retain their owning VM."""

    def __init__(self, native):
        self._native = native
        self._functions = {}

    def __getitem__(self, name):
        if name not in self._functions:
            self._functions[name] = self._native.function(name)
        return self._functions[name]

    def __getattr__(self, name):
        return self[name]

    def __dir__(self):
        return sorted(set(super().__dir__()) | set(self._native.functions))


class VM:
    """Owns a compiler snapshot and synchronous, headless HGVM interpreter."""

    def __init__(self, settings=None):
        settings = {} if settings is None else dict(settings)
        unknown = set(settings) - {"search_paths", "working_directory", "backend"}
        if unknown:
            raise TypeError(f"Unknown slangvm settings: {sorted(unknown)}")
        self._directory = Path(settings.get("working_directory", Path.cwd())).resolve()
        paths = [str((self._directory / p).resolve()) for p in settings.get("search_paths", [])]
        self._native = _slangvm.VM(paths, str(self._directory), settings.get("backend", "none"))

    def load(self, path):
        """Compile the file's concrete [host] exports without executing them."""
        return Module(self._native.load(str((self._directory / path).resolve())))

    @property
    def is_closed(self):
        return self._native.is_closed

    def close(self):
        """Invalidate functions and values and release native payloads immediately."""
        self._native.close()

    def __enter__(self):
        if self.is_closed:
            raise RuntimeError("The slangvm VM is closed")
        return self

    def __exit__(self, *_):
        self.close()


def init(settings=None):
    """Create an independent VM; set backend to e.g. 'vulkan' to enable GPU operations.

    The default backend, 'none', creates no device. 'default' lets RHI choose a device.
    Optional search_paths and working_directory settings control source loading.
    """
    return VM(settings)
