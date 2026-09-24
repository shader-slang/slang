#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""Exercise finite batches with a controlled peer and optional real test-server hooks.

SLANG_COMPLEX_TEST_SERVER selects the real server for integration tests.
"""

import argparse
import importlib.util
import json
import os
from pathlib import Path
import signal
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("complex_runner", Path(__file__).with_name("run-complex-corpus.py"))
RUNNER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(RUNNER)
BATCH = RUNNER.load_batch_helpers()


def fake_output(arguments):
    entry = arguments[arguments.index("-entry") + 1] if "-entry" in arguments else "main"
    return f".target sm_80\n.entry {entry}() {{}}\n"


def fake_peer():
    """Deliberately violate wire contracts independently of the client's framing code."""
    mode = os.environ.get("FAKE_MODE", "normal")
    ordinal = 0
    while True:
        header = sys.stdin.buffer.readline()
        if not header:
            return
        size = int(header.split(b":")[1])
        assert sys.stdin.buffer.readline() == b"\r\n"
        request = json.loads(sys.stdin.buffer.read(size))
        if request["method"] == "quit":
            if mode == "shutdown-hang":
                time.sleep(30)
            if mode == "shutdown-exit":
                sys.exit(7)
            return
        ordinal += 1
        if mode == "pipe-child":
            child = subprocess.Popen([sys.executable, "-c", "import time; time.sleep(30)"])
            print(child.pid, file=sys.stderr, flush=True)
            return
        args = request["params"]["args"]
        if mode == "timeout" or mode == "second-timeout" and ordinal == 2:
            time.sleep(30)
        if mode == "crash" and ordinal == 2:
            sys.exit(13)
        if mode == "signal" and ordinal == 2:
            os.kill(os.getpid(), signal.SIGKILL)
        if "-o" in args and mode != "no-output":
            data = fake_output(args)
            if mode == "mismatch":
                data += "// different\n"
            Path(args[args.index("-o") + 1]).write_text(data)
        result = {"stdOut": "", "stdError": "", "debugLayer": "", "result": 0, "returnCode": 0}
        if mode == "compile-error":
            result.update(result=-2147467259, returnCode=1, stdError="error[E52017]: unsupported\n")
        if mode == "bool-result":
            result["returnCode"] = False
        if mode == "missing-result":
            del result["result"]
        if mode == "inconsistent-result":
            result["result"] = -1
        reply = {"jsonrpc": "2.0", "id": request["id"], "result": result}
        if mode == "wrong-id":
            reply["id"] += 1
        if mode == "string-id":
            reply["id"] = str(reply["id"])
        if mode == "rpc-error":
            reply = {"jsonrpc": "2.0", "id": request["id"], "error": {"code": -32602}}
        body = json.dumps(reply).encode()
        if mode == "invalid-json":
            body = b"{oops}"
        if mode == "deep-json":
            body = b"[" * 2000 + b"0" + b"]" * 2000
        if mode == "duplicate-json":
            body = body[:-1] + b',"id":1}'
        frame = b"Content-Length: " + str(len(body)).encode() + b"\r\n\r\n" + body
        if mode == "oversized":
            frame = b"Content-Length: 999999999\r\n\r\n"
        if mode == "garble":
            frame = b"not-a-header\r\n\r\n"
        if mode == "duplicate-length":
            frame = b"Content-Length: 1\r\nContent-Length: 1\r\n\r\n0"
        if mode == "header-limit":
            frame = b"a" * (BATCH.MAX_HEADER + 1)
        if mode == "truncated":
            sys.stdout.buffer.write(frame[:-3])
            sys.stdout.buffer.flush()
            return
        if mode == "stderr-flood":
            sys.stderr.buffer.write(b"diagnostic\n" * 200000)
            sys.stderr.buffer.flush()
        if mode == "fragmented":
            for byte in frame:
                sys.stdout.buffer.write(bytes([byte]))
                sys.stdout.buffer.flush()
        else:
            sys.stdout.buffer.write(frame)
            sys.stdout.buffer.flush()


class BatchTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.directory = Path(self.temp.name)

    def run_peer(self, mode="normal", count=3, timeout=1):
        report = BATCH.run_batch([sys.executable, str(Path(__file__).resolve()), "--fake-peer"],
                                 [["-version"]] * count, self.directory / mode,
                                 dict(os.environ, FAKE_MODE=mode), timeout, shutdown_timeout=.3)
        if "pid" in report and os.name != "nt":
            with self.assertRaises(ProcessLookupError):
                os.kill(report["pid"], 0)
        self.assertNotEqual(report["status"], "cleanup-failed")
        return report

    def test_fragmented_success_and_stderr_drain(self):
        for mode in ["normal", "fragmented", "stderr-flood"]:
            with self.subTest(mode=mode):
                r = self.run_peer(mode)
                self.assertEqual(r["status"], "completed")
                self.assertEqual([c["status"] for c in r["cells"]], ["completed"] * 3)
                self.assertEqual(r["process_return_code"], 0)
                self.assertGreater(r["elapsed_seconds"], sum(c["elapsed_seconds"] for c in r["cells"]))
        self.assertGreater((self.directory / "stderr-flood/stderr.log").stat().st_size, 1000000)

    def test_protocol_violations_fail_active_and_preserve_suffix(self):
        for mode in ["wrong-id", "string-id", "rpc-error", "invalid-json", "duplicate-json", "deep-json",
                     "oversized", "garble", "duplicate-length", "header-limit", "truncated",
                     "bool-result", "missing-result", "inconsistent-result"]:
            with self.subTest(mode=mode):
                r = self.run_peer(mode)
                self.assertEqual(r["status"], "failed")
                self.assertEqual([c["status"] for c in r["cells"]],
                                 ["protocol-error", "incomplete", "incomplete"])
                self.assertGreater(Path(r["stdout_log"]).stat().st_size, 0)

    def test_startup_and_later_deadlines(self):
        for mode, expected in [("timeout", ["timeout", "incomplete", "incomplete"]),
                               ("second-timeout", ["completed", "timeout", "incomplete"])]:
            r = self.run_peer(mode, timeout=.2)
            self.assertEqual([c["status"] for c in r["cells"]], expected)
            self.assertLess(r["elapsed_seconds"], 2)
            self.assertEqual(r["retries"], 0)

    def test_crash_preserves_completed_prefix_and_exit_or_signal(self):
        for mode, code in [("crash", 13), ("signal", -9)]:
            if mode == "signal" and os.name == "nt":
                continue
            r = self.run_peer(mode)
            self.assertEqual([c["status"] for c in r["cells"]],
                             ["completed", "server-exited", "incomplete"])
            self.assertEqual(r["process_return_code"], code)

    def test_shutdown_is_required_even_after_all_replies(self):
        for mode, status in [("shutdown-hang", "shutdown-timeout"), ("shutdown-exit", "shutdown-failed")]:
            r = self.run_peer(mode)
            self.assertEqual(r["status"], status)
            self.assertTrue(all(c["status"] == "completed" for c in r["cells"]))

    def test_compile_failure_preserves_exact_result(self):
        r = self.run_peer("compile-error")
        self.assertEqual(r["status"], "completed")
        self.assertEqual(r["cells"][0]["return_code"], 1)
        self.assertEqual(r["cells"][0]["result"], -2147467259)

    @unittest.skipIf(os.name == "nt", "POSIX owned process group")
    def test_exited_leader_with_pipe_holding_child_is_cleaned_up(self):
        r = self.run_peer("pipe-child", timeout=.2)
        self.assertEqual(r["cells"][0]["status"], "timeout")
        self.assertLess(r["elapsed_seconds"], 2)
        pid = int(Path(r["stderr_log"]).read_text().strip())
        stat = Path(f"/proc/{pid}/stat")
        if stat.exists():
            self.assertEqual(stat.read_text().split()[2], "Z")

    def test_launch_failure_and_bound(self):
        r = BATCH.run_batch([str(self.directory / "missing")], [["-version"]] * 2,
                            self.directory / "launch", os.environ, .2)
        self.assertEqual([c["status"] for c in r["cells"]], ["launch-failed", "incomplete"])
        with self.assertRaises(ValueError):
            BATCH.run_batch([], [[]] * 7, self.directory, os.environ, 1)

    @unittest.skipUnless(os.environ.get("SLANG_COMPLEX_TEST_SERVER"), "real server path not supplied")
    def test_real_server_success_and_existing_failure_hooks(self):
        for hook, status, code in [(None, "completed", 0),
                                   ("DIE", "server-exited", 1),
                                   ("KILL", "server-exited", -9 if os.name != "nt" else 1),
                                   ("GARBLE", "protocol-error", None)]:
            environment = dict(os.environ)
            if hook:
                environment[f"SLANG_TEST_SERVER_{hook}_ON_REQUEST"] = "2"
            r = BATCH.run_batch([os.environ["SLANG_COMPLEX_TEST_SERVER"]], [["-version"]] * 3,
                                self.directory / str(hook), environment, 5)
            self.assertEqual(r["cells"][0]["status"], "completed")
            self.assertEqual(r["cells"][1]["status"], status)
            if hook:
                self.assertEqual(r["cells"][2]["status"], "incomplete")
            else:
                self.assertEqual(r["status"], "completed")
            if code is not None:
                self.assertEqual(r["process_return_code"], code)


@unittest.skipIf(os.name == "nt", "fixture executable scripts require POSIX")
class RunnerTests(unittest.TestCase):
    # Reuse only the temporary directory setup, not BatchTests' protocol cases.
    def setUp(self):
        BatchTests.setUp(self)
        test_file = str(Path(__file__).resolve())
        import shlex
        for name, flag in [("slangc", "--fake-compiler"), ("test-server", "--fake-peer"), ("ptxas", "--fake-compiler")]:
            path = self.directory / name
            path.write_text(f"#!/bin/sh\nexec {shlex.quote(sys.executable)} {shlex.quote(test_file)} {flag} \"$@\"\n")
            path.chmod(0o755)
        self.args = argparse.Namespace(output=self.directory / "outputs", slangc=self.directory / "slangc",
                                       test_server=self.directory / "test-server", ptxas=self.directory / "ptxas",
                                       warmup=0, samples=1, timeout=2)
        self.toolkit = RUNNER.load_toolkit_helpers()
        self.workloads = {"fixture": {"source": "tests/cuda/complex/tiled_brass_material.slang", "stage": "compute"}}
        self.cells = [{"id": str(i), "entry": "main", "architecture": 80, "backend": backend,
                       "optimization": opt, "workload": "fixture", "status": "pending"}
                      for i, (backend, opt) in enumerate(RUNNER.MODES)]

    def run_shared(self, mode="normal", **env):
        report = {"cells": self.cells, "artifact_sha256": {}}
        RUNNER.assess_shared(report, self.workloads, self.args, self.toolkit,
                             dict(os.environ, FAKE_MODE=mode, **env), lambda: None)
        return report

    def test_shared_references_and_primary_counts(self):
        report = self.run_shared()
        self.assertEqual(len(report["cells"]), 3)
        self.assertEqual(report["fresh_reference_counts"], {"requested": 3, "passed": 3, "attempted": 3})
        self.assertTrue(all(c["status"] == "passed" for c in self.cells))
        self.assertTrue(all(c["phase_median_ms"] is None for c in self.cells))
        self.assertEqual(report["batch_validation_status"], "passed")
        self.assertGreater(report["fresh_reference_seconds"], 0)

    def test_default_fresh_schema_and_exact_options(self):
        for cell in self.cells:
            RUNNER.assess_cell(cell, self.workloads["fixture"], self.args, self.toolkit, os.environ)
            self.assertEqual(cell["status"], "passed")
            self.assertNotIn("successful_service_median_seconds", cell)
            expected = RUNNER.compile_command(cell, self.workloads["fixture"], self.args)
            self.assertEqual(cell["attempts"][0]["command"][:-2], expected)

    def test_unequal_batches_aggregate_complete_samples(self):
        self.cells = [dict(self.cells[i % 3], id=str(i)) for i in range(7)]
        self.args.samples = 2
        r = self.run_shared()
        self.assertEqual([len(b["cells"]) for b in r["batches"]], [6, 1, 6, 1])
        self.assertEqual(len({b["pid"] for b in r["batches"]}), 4)
        totals = [sum(b["elapsed_seconds"] for b in r["batches"] if b["sample_index"] == i)
                  for i in range(2)]
        self.assertAlmostEqual(r["sample_compile_lifecycle_median_seconds"], sum(totals) / 2)
        self.assertTrue(all(c["status"] == "passed" and len(c["attempts"]) == 2 for c in self.cells))

    def test_failed_reference_blocks_all_shared_work(self):
        r = self.run_shared(FAKE_REFERENCE_FAILURE="1")
        self.assertEqual(r["batch_validation_status"], "reference-failed")
        self.assertEqual(r["batches"], [])
        self.assertTrue(all(c["status"] == "incomplete" for c in self.cells))
        self.assertTrue(all(c["status"] == "compile-failed" for c in r["fresh_references"]))

    def test_mismatch_and_stale_outputs_cannot_pass(self):
        for mode in ["mismatch", "no-output"]:
            with self.subTest(mode=mode):
                for cell in self.cells:
                    path = self.args.output / cell["id"] / "attempt-0.ptx"
                    path.parent.mkdir(parents=True, exist_ok=True)
                    path.write_text(fake_output(["-entry", "main"]))
                r = self.run_shared(mode)
                self.assertTrue(all(c["status"] == "artifact-failed" for c in self.cells))
                self.assertEqual(r["batch_validation_status"], "failed")
                self.assertIsNone(r["sample_compile_lifecycle_median_seconds"])

    def test_crash_and_shutdown_failures_cannot_be_blessed(self):
        r = self.run_shared("crash")
        self.assertEqual([c["status"] for c in self.cells], ["passed", "infrastructure-failed", "incomplete"])
        self.assertEqual(r["batch_validation_status"], "failed")
        r = self.run_shared("shutdown-exit")
        self.assertTrue(all(c["status"] == "passed" for c in self.cells))
        self.assertEqual(r["batch_validation_status"], "failed")
        self.assertIsNone(r["sample_compile_lifecycle_median_seconds"])
        self.assertTrue(all(c["successful_service_median_seconds"] is None for c in self.cells))


if __name__ == "__main__":
    if "--fake-peer" in sys.argv:
        fake_peer()
    elif "--fake-compiler" in sys.argv:
        if os.environ.get("FAKE_REFERENCE_FAILURE"):
            print("error: reference failure", file=sys.stderr)
            sys.exit(1)
        args = sys.argv[2:]
        Path(args[args.index("-o") + 1]).write_text(fake_output(args))
    else:
        unittest.main()
