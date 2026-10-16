"""Exercise conversion, API limits and credential-safe submission failures."""

import copy
import io
import json
import subprocess
import sys
import tempfile
import unittest
import urllib.error
from pathlib import Path
from unittest.mock import patch, MagicMock

import benchview


class BenchViewTests(unittest.TestCase):
    """Keep the reporting adapter independent of a live server or write key."""

    def setUp(self):
        """Use overlapping timers to exercise the shared allocation model."""
        self.record = {"workload": "mdl_dxr", "mode": "cli", "size": 1, "ok": True,
                       "timers": {"compileInner": {"samples": [10, 20]},
                                  "linkAndOptimizeIR": {"samples": [12, 25]}}}
        self.meta = {"commit": "a" * 40, "runner": "perf-runner", "kind": "daily"}

    def convert(self, records=None, **kwargs):
        """Export a fixed execution so equality checks cover replay identity."""
        return benchview.submissions(records or [self.record], self.meta, "Windows",
                                     "2026-10-14T05:00:00Z", "job-1", **kwargs)

    def test_release_and_daily(self):
        """Release metadata belongs to the Slang version, and daily runs omit it."""
        daily = self.convert()[0]
        release = self.convert(release={"label": "v2026.19"})[0]
        self.assertEqual(daily["project"], {"id": "slang", "name": "Slang",
                         "metadata": {"defaultBranch": "master", "breakdownTooltipMaxItems": 1}})
        self.assertNotIn("metadata", daily["run"])
        self.assertEqual(release["run"]["metadata"]["release"]["label"], "v2026.19")
        self.assertNotEqual(daily["run"]["key"], release["run"]["key"])
        self.assertEqual(self.convert()[0], daily)

    def test_samples_and_breakdown(self):
        """Allocated phases conserve each root sample despite overlapping timers."""
        metrics = self.convert()[0]["observations"][0]["metrics"]
        root = next(m for m in metrics if m["name"] == "compileInner")
        self.assertEqual(root["distribution"], {"samples": [10, 20]})
        components = root["breakdown"]["components"]
        for i, total in enumerate([10, 20]):
            self.assertAlmostEqual(sum(c["distribution"]["samples"][i] for c in components), total)

    def test_unaligned_samples_omit_only_breakdown(self):
        """Keep passed measurements without inventing phase sample alignment."""
        self.record["timers"]["linkAndOptimizeIR"]["samples"] = [12]
        with patch('sys.stderr', new_callable=io.StringIO) as stderr:
            observation = self.convert()[0]["observations"][0]
        self.assertEqual(observation["status"], "passed")
        root = next(m for m in observation["metrics"] if m["name"] == "compileInner")
        self.assertNotIn("breakdown", root)
        self.assertEqual(root["distribution"]["samples"], [10, 20])
        self.assertIn("breakdownOmitted", root["metadata"])
        self.assertIn("omitted unaligned breakdown", stderr.getvalue())

    def test_source_emission_tree_and_priority(self):
        """Source emission contains linking, including when the metric cap is reached."""
        values = {"compileInner": 100, "generateOutput": 90,
                  "emitEntryPointsSourceFromIR": 80, "linkAndOptimizeIR": 60}
        self.record["timers"] = {name: {"samples": [value, value * 2]}
                                 for name, value in values.items()}
        self.record["timers"].update({f"aaa{i}": {"samples": [1, 2]} for i in range(80)})
        metrics = self.convert()[0]["observations"][0]["metrics"]
        self.assertIn("emitEntryPointsSourceFromIR", [m["name"] for m in metrics])
        root = next(m for m in metrics if m["name"] == "compileInner")
        actual = {c["name"]: c["distribution"]["samples"]
                  for c in root["breakdown"]["components"]}
        for i in range(2):
            expected = benchview.buckets({name: value * (i + 1) for name, value in values.items()})
            self.assertEqual({name: samples[i] for name, samples in actual.items()}, expected)
        self.assertEqual(actual["emitEntryPointsSourceFromIR (self)"], [20, 40])

    def test_component_id_collision(self):
        """Reject normalized component collisions before contacting the server."""
        tree = ("root", [("Foo", []), ("foo", [])])
        timers = {"root": {"samples": [10]}, "Foo": {"samples": [2]},
                  "foo": {"samples": [3]}}
        with self.assertRaisesRegex(ValueError, "component names collide"):
            benchview.phase_breakdown(timers, tree)

    def test_runner_os_mismatch(self):
        """A changed runner platform cannot silently inherit the Windows label."""
        self.meta["runner"] = "Linux-x86_64-cpu"
        with self.assertRaisesRegex(ValueError, "OS disagrees"):
            self.convert()

    def test_malformed_inputs_have_single_line_errors(self):
        """CLI input errors should not leak tracebacks into the nightly log."""
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'meta.json').write_text(json.dumps(self.meta))
            malformed = copy.deepcopy(self.record)
            malformed['timers']['compileInner'] = 5
            for records in ([None], [malformed]):
                (root / 'results.json').write_text(json.dumps(records))
                result = subprocess.run(
                    [sys.executable, str(Path(benchview.__file__)),
                     '--results', str(root / 'results.json'), '--meta', str(root / 'meta.json'),
                     '--os', 'Windows', '--observed-at', '2026-10-15T00:00:00Z',
                     '--execution-id', 'test', '--output', str(root / 'out')],
                    capture_output=True, text=True)
                self.assertEqual(result.returncode, 1)
                self.assertEqual(len(result.stderr.splitlines()), 1)
                self.assertIn('BenchView export failed:', result.stderr)
        with self.assertRaises(ValueError):
            benchview.timestamp(123)

    def test_zero_root(self):
        """A zero-duration root still exports a valid complete breakdown."""
        self.record["timers"] = {"compileInner": {"samples": [0, 0]}}
        metric = self.convert()[0]["observations"][0]["metrics"][0]
        components = metric["breakdown"]["components"]
        self.assertEqual(len(components), 1)
        self.assertEqual(components[0]["distribution"]["samples"], [0, 0])

    def test_summary_only_rejected(self):
        """Old summaries cannot recover missing raw samples or maxima."""
        self.record["timers"]["compileInner"] = {"median": 10, "n": 2}
        with self.assertRaisesRegex(ValueError, "samples"):
            self.convert()

    def test_metric_limit_retains_source(self):
        """Overflow timers remain in the original record and are listed explicitly."""
        self.record["timers"].update({f"extra{i}": {"samples": [1, 2]} for i in range(80)})
        observation = self.convert()[0]["observations"][0]
        self.assertEqual(len(observation["metrics"]), 64)
        self.assertEqual(len(observation["metadata"]["ungraphedTimers"]), 18)
        self.assertIn('extra79', observation["artifacts"][0]["content"]["text"])

    def test_batches_keep_release(self):
        """The 501st observation starts a new batch with identical run metadata."""
        records = [dict(self.record, size=i) for i in range(501)]
        payloads = self.convert(records, release={"label": "v2026.19"})
        self.assertEqual([len(p["observations"]) for p in payloads], [500, 1])
        self.assertEqual(payloads[0]["run"], payloads[1]["run"])
        self.assertNotEqual(payloads[0]["idempotencyKey"], payloads[1]["idempotencyKey"])

    def test_failed_workload(self):
        """Failed workloads can be reported without timing samples."""
        self.record.update(ok=False, timers={})
        observation = self.convert()[0]["observations"][0]
        self.assertEqual(observation["status"], "failed")
        self.assertIn("error", observation)

    def test_failed_partial_samples_do_not_abort_nightly(self):
        """One failed workload exports its source alongside unaffected successful results."""
        failed = copy.deepcopy(self.record)
        failed.update(workload="failed", ok=False, error="sample failed")
        failed['timers']['linkAndOptimizeIR']['samples'] = [12]
        observations = self.convert([self.record, failed])[0]['observations']
        self.assertEqual([o['status'] for o in observations], ['passed', 'failed'])
        self.assertTrue(observations[0]['metrics'])
        self.assertEqual(observations[1]['metrics'], [])
        self.assertEqual(json.loads(observations[1]['artifacts'][0]['content']['text']), failed)

    def test_invalid_dates_and_duplicates(self):
        """Reject impossible release dates and ambiguous observation identities."""
        with self.assertRaises(ValueError):
            self.convert(release={"label": "v1", "releasedAt": "2026-02-30T00:00:00Z"})
        with self.assertRaisesRegex(ValueError, "Duplicate"):
            self.convert([self.record, copy.deepcopy(self.record)])

    @patch("benchview.time.sleep")
    @patch("benchview.urllib.request.build_opener")
    def test_retry_uses_same_request(self, build_opener, sleep):
        """An uncertain transport outcome retries identical bytes and identity."""
        opener = build_opener.return_value
        response = MagicMock()
        response.__enter__.return_value.status = 201
        opener.open.side_effect = [urllib.error.URLError("offline"), response]
        benchview.submit(self.convert()[0], "https://example.test/benchview", "secret")
        self.assertEqual(opener.open.call_count, 2)
        self.assertIs(opener.open.call_args_list[0].args[0],
                      opener.open.call_args_list[1].args[0])
        self.assertEqual(opener.open.call_args.args[0].full_url,
                         "https://example.test/benchview/api/v1/submissions")

    @patch("benchview.urllib.request.build_opener")
    def test_auth_error_not_retried_or_echoed(self, build_opener):
        """Neither an error body nor its URL should expose server-reflected credentials."""
        build_opener.return_value.open.side_effect = urllib.error.HTTPError(
            "http://example.test/secret", 401, "secret", {}, io.BytesIO(b"secret"))
        with self.assertRaisesRegex(RuntimeError, "^BenchView submission failed: HTTP 401$"):
            benchview.submit(self.convert()[0], "https://example.test", "secret")
        self.assertEqual(build_opener.return_value.open.call_count, 1)
        self.assertIsNone(benchview.NoRedirect().redirect_request(None, None, 302, "", {},
                                                                 "http://other.test"))

    @patch("benchview.urllib.request.build_opener")
    def test_http_requires_explicit_opt_in(self, build_opener):
        """Reject cleartext before creating a credential-bearing network request."""
        with self.assertRaisesRegex(ValueError, "unencrypted"):
            benchview.submit(self.convert()[0], "http://example.test", "secret")
        build_opener.assert_not_called()
        build_opener.return_value.open.return_value.__enter__.return_value.status = 201
        benchview.submit(self.convert()[0], "http://example.test", "secret", allow_insecure=True)
        build_opener.return_value.open.assert_called_once()


if __name__ == "__main__":
    unittest.main()
