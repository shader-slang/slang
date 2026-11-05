import importlib.util
from pathlib import Path
import unittest
from urllib.error import HTTPError

spec = importlib.util.spec_from_file_location(
    "wait_for_pages", Path(__file__).parents[1] / "wait-for-pages.py")
checker = importlib.util.module_from_spec(spec)
spec.loader.exec_module(checker)
SHA = "a" * 40
OLD = "b" * 40


def run(status="completed", conclusion="success", sha=SHA, identity=1):
    return {"id": identity, "head_sha": sha, "status": status,
            "conclusion": conclusion, "html_url": f"https://github.com/example/runs/{identity}"}


class PublicationTests(unittest.TestCase):
    def check(self, responses, timeout=3, sha=SHA):
        clock = [0]
        self.urls = []
        sequence = iter(responses)

        def fetch(url):
            self.urls.append(url)
            response = next(sequence)
            if isinstance(response, Exception):
                raise response
            return response

        def sleep(seconds):
            clock[0] += seconds

        return checker.wait_for_pages(sha, timeout=timeout, interval=1, fetch=fetch,
                                      now=lambda: clock[0], sleep=sleep)

    def test_waits_for_run_and_served_commit(self):
        self.check([
            {"workflow_runs": []},
            {"workflow_runs": [run("in_progress", None)]},
            {"workflow_runs": [run()]}, {"report_commit": SHA},
        ])
        self.assertIn("head_sha=" + SHA, self.urls[0])
        self.assertTrue(self.urls[-1].startswith(checker.SITE))

    def test_failed_deployment_fails_immediately(self):
        for conclusion in ("failure", "cancelled", "timed_out", "skipped", "neutral"):
            with self.subTest(conclusion=conclusion):
                with self.assertRaisesRegex(RuntimeError, conclusion):
                    self.check([{"workflow_runs": [run(conclusion=conclusion)]}])
                self.assertEqual(len(self.urls), 1)

    def test_old_success_does_not_count(self):
        with self.assertRaisesRegex(TimeoutError, "No Pages run found"):
            self.check([{"workflow_runs": [run(sha=OLD)]}] * 3)

    def test_stale_site_does_not_count(self):
        with self.assertRaisesRegex(TimeoutError, "older report"):
            self.check([{"workflow_runs": [run()]}, {"report_commit": OLD}] * 3)

    def test_site_can_catch_up_after_success(self):
        self.check([{"workflow_runs": [run()]}, {"report_commit": OLD},
                    {"workflow_runs": [run()]}, {"report_commit": SHA}])

    def test_network_error_retries_without_claiming_success(self):
        error = HTTPError(checker.API, 503, "Unavailable", {}, None)
        self.check([error, {"workflow_runs": [run()]}, {"report_commit": SHA}])
        with self.assertRaisesRegex(TimeoutError, "Cannot verify publication"):
            self.check([error] * 3)

    def test_newest_run_wins(self):
        with self.assertRaisesRegex(RuntimeError, "failure"):
            self.check([{"workflow_runs": [run(), run(conclusion="failure", identity=2)]}])

    def test_invalid_sha_never_calls_network(self):
        with self.assertRaisesRegex(ValueError, "full report commit SHA"):
            self.check([], sha="main")
        self.assertEqual(self.urls, [])


if __name__ == "__main__":
    unittest.main()
