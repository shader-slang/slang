"""GPU-free regression checks for result validity and comparison boundaries."""
import copy
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import perf


class RuntimePerfTests(unittest.TestCase):
    def fixture(self):
        return dict(schema_version=2, workload_version=1, environment={"driver_id": "driver-1"},
                    generator_sha256="generator", runner_source_sha256="runner", config=dict(perf.DEFAULTS),
                    runs=[dict(workload="arithmetic_basic", status="ok", validated=True, size=4, elements=65536,
                               width=256, batch=8, measurement_mode="batched_throughput",
                               source_sha256="source", adapter="gpu", backend="vulkan",
                               rhi_revision="rhi", timestamp_frequency=1000000,
                               compiler="baseline", timers={"gpu_total_ms": {"median": 10.0}})])

    def compare(self, a, b):
        with tempfile.TemporaryDirectory() as directory:
            paths = [Path(directory) / "a.json", Path(directory) / "b.json"]
            for path, result in zip(paths, [a, b]):
                perf.write_json(path, result)
            return perf.compare(*paths)

    def test_compiler_change_is_the_comparison_variable(self):
        a = self.fixture()
        b = copy.deepcopy(a)
        b["runs"][0]["compiler"] = "candidate"
        b["runs"][0]["timers"]["gpu_total_ms"]["median"] = 12
        self.assertEqual(self.compare(a, b)[0]["ratio"], 1.2)

    def test_noisy_phase_is_advisory_for_stable_total(self):
        a = self.fixture()
        a["runs"][0]["timers"]["gpu_main_ms"] = {"median": 8.0}
        b = copy.deepcopy(a)
        b["runs"][0]["noisy_phases"] = ["gpu_main_ms"]
        result = self.compare(a, b)[0]
        self.assertEqual(result["ratio"], 1.0)
        self.assertTrue(result["phases"]["gpu_main_ms"]["above_spread_target"])
        self.assertFalse(result["phases"]["gpu_total_ms"]["above_spread_target"])

    def test_phase_policy_change_requires_new_baseline(self):
        a = self.fixture()
        b = copy.deepcopy(a)
        b["config"]["strict_phases"] = True
        with self.assertRaisesRegex(ValueError, "policies"):
            self.compare(a, b)

    def test_rejects_mismatched_work_and_environment(self):
        for key, value in [("elements", 131072), ("batch", 16), ("source_sha256", "changed"),
                           ("adapter", "different GPU"), ("rhi_revision", "changed"),
                           ("scene", {"version": 2})]:
            with self.subTest(key=key):
                a = self.fixture()
                b = copy.deepcopy(a)
                b["runs"][0][key] = value
                with self.assertRaisesRegex(ValueError, key):
                    self.compare(a, b)

    def test_rejects_missing_or_invalid_results(self):
        for status in ("unstable", "unsupported", "failed"):
            a = self.fixture()
            b = copy.deepcopy(a)
            b["runs"][0]["status"] = status
            with self.assertRaisesRegex(ValueError, "stable"):
                self.compare(a, b)
        a = self.fixture()
        b = copy.deepcopy(a)
        b["runs"] = []
        with self.assertRaisesRegex(ValueError, "coverage"):
            self.compare(a, b)

    def test_unknown_driver_cannot_be_assumed_equal(self):
        a = self.fixture()
        a["environment"]["driver_id"] = ""
        with self.assertRaisesRegex(ValueError, "driver"):
            self.compare(a, a)

    def test_optix_version_is_part_of_comparison_contract(self):
        a = self.fixture()
        a['runs'][0].update(workload='raytrace_pipeline', backend='cuda', optix_version=90000)
        b = copy.deepcopy(a)
        for version in (80100, None):
            b['runs'][0]['optix_version'] = version
            with self.subTest(version=version), self.assertRaisesRegex(ValueError, 'optix_version'):
                self.compare(a, b)

    def test_cuda_ray_paired_compile_uses_measured_headers_and_raygen_architecture(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            headers = root / 'OptiX headers'
            headers.mkdir()
            for name in ('optix.h', 'optix_device.h'):
                (headers / name).write_text('// pinned header')
            (root / 'rayGenMain.target-code').write_text('.target sm_89')
            source = dict(directory=str(root), files=['main.slang'])
            with patch.object(perf.bench, 'run_spec', return_value={'ok': True}) as compile:
                perf.compile_case('slangc', 'raytrace_pipeline', dict(perf.DEFAULTS, backend='cuda'),
                                  source, root, dict(optix_headers=str(headers)))
            spec = compile.call_args.args[1]
            self.assertIn('-Xnvrtc', spec.extra_flags)
            self.assertIn('-I' + str(headers), spec.extra_flags)
            self.assertIn('cuda_sm_8_9', spec.extra_flags)
            self.assertTrue(spec.downstream_required)
            self.assertEqual(spec.name, 'runtime_raytrace_pipeline')

    def test_cuda_ray_paired_compile_rejects_missing_headers(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with self.assertRaisesRegex(ValueError, 'measured OptiX headers'):
                perf.compile_case('slangc', 'raytrace_pipeline', dict(perf.DEFAULTS, backend='cuda'),
                                  dict(directory=directory, files=['main.slang']), root)

    def test_rejects_incomplete_and_nonfinite_samples(self):
        for samples in ([], [{"gpu_total_ms": float("nan")}], [{"gpu_total_ms": -1}],
                        [{"gpu_total_ms": 1}, {"host_wait_ms": 1}]):
            with self.subTest(samples=samples), self.assertRaises(ValueError):
                perf.aggregate(samples)

    def test_preserves_raw_samples(self):
        values = [1.1234567, 1.2345678, 1.3456789]
        result = perf.aggregate([{"gpu_total_ms": v} for v in values])
        self.assertEqual(result["gpu_total_ms"]["samples"], values)

    def test_every_compile_entry_uses_identical_runtime_source(self):
        for case, family in perf.workloads.CASES.items():
            for complexity in (1, 4, 16):
                files = perf.workloads.generate(case, complexity)
                self.assertEqual(files, perf.workloads.generate(case, complexity))
                for filename, source in files.items():
                    name = "runtime_" + case + ("_" + Path(filename).stem if family == "image" else "")
                    spec = perf.manifest.BY_NAME[name]
                    self.assertTrue(spec.opt_in)
                    self.assertEqual(spec.gen(complexity), {filename: source})

    def test_saved_calibration_rejects_changed_source_or_policy(self):
        config = dict(perf.DEFAULTS)
        corpus = {"arithmetic_basic": {"source_sha256": "a"}}
        saved = dict(schema_version=2, workload_version=perf.workloads.VERSION,
                     environment={}, policy=perf.policy(config),
                     runner_source_sha256=perf.runner_source_hash(),
                     cases={"arithmetic_basic": {"source_sha256": "a"}})
        perf.check_calibration(saved, {}, config, corpus)
        strict = dict(config, strict_phases=True)
        with self.assertRaisesRegex(ValueError, "policy"):
            perf.check_calibration(saved, {}, strict, corpus)
        with self.assertRaisesRegex(ValueError, "source"):
            perf.check_calibration(saved, {}, config, {"arithmetic_basic": {"source_sha256": "b"}})
        config["target_ms"] *= 2
        with self.assertRaisesRegex(ValueError, "policy"):
            perf.check_calibration(saved, {}, config, corpus)

    def test_old_calibration_schema_requires_fresh_measurements(self):
        with self.assertRaisesRegex(ValueError, "schema"):
            perf.check_calibration(dict(schema_version=1), {}, dict(perf.DEFAULTS), {})

    def test_comparison_rejects_old_timer_schema(self):
        a = self.fixture()
        b = copy.deepcopy(a)
        b["schema_version"] = 1
        with self.assertRaisesRegex(ValueError, "schema_version"):
            self.compare(a, b)

    def test_invalid_sizes_fail_before_native_execution(self):
        class Args:
            config = None
        args = Args()
        for size in (-1, 123, 65537, 33554432):
            args.elements = size
            with self.subTest(size=size), self.assertRaises(ValueError):
                perf.settings(args)


if __name__ == "__main__":
    unittest.main()
