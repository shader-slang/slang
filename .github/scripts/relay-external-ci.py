#!/usr/bin/env python3
import json
import os
import sys
import time
import urllib.error
import urllib.parse
import urllib.request


TERMINAL_STATUSES = {"success", "failed", "canceled", "skipped", "manual"}


def main():
    github_token = required_env("GITHUB_TOKEN")
    repository = required_env("GITHUB_REPOSITORY")
    run_id = required_env("GITHUB_RUN_ID")
    artifact_name = required_env("EXTERNAL_CI_ARTIFACT_NAME")

    ci_api_base_url = required_env("EXTERNAL_CI_API_BASE_URL").rstrip("/")
    ci_project = required_env("EXTERNAL_CI_PROJECT")
    ci_ref = required_env("EXTERNAL_CI_REF")
    ci_token = required_env("EXTERNAL_CI_TOKEN")
    poll_interval = int(os.environ.get("EXTERNAL_CI_POLL_INTERVAL_SECONDS", "30"))
    timeout_seconds = int(os.environ.get("EXTERNAL_CI_TIMEOUT_SECONDS", "7200"))

    print("Resolving build artifact")
    artifact_url = find_github_artifact(repository, run_id, artifact_name, github_token)

    variables = relay_variables()
    add_variable(
        variables,
        os.environ.get("EXTERNAL_CI_ARTIFACT_URL_VARIABLE", "CI_SLANG_ARTIFACT_URL"),
        artifact_url,
    )
    add_variable(
        variables,
        os.environ.get("EXTERNAL_CI_ARTIFACT_TOKEN_VARIABLE", "CI_SLANG_ARTIFACT_TOKEN"),
        github_token,
    )

    print("Starting external CI")
    pipeline = trigger_pipeline(ci_api_base_url, ci_project, ci_ref, ci_token, variables)
    pipeline_id = pipeline.get("id")
    if not pipeline_id:
        raise RuntimeError("External CI response did not include a pipeline id")

    status = wait_for_pipeline(
        ci_api_base_url,
        ci_project,
        pipeline_id,
        ci_token,
        poll_interval,
        timeout_seconds,
    )
    if status == "success":
        print("External CI completed successfully")
        return 0

    print(f"External CI completed with status: {status}", file=sys.stderr)
    return 1


def required_env(name):
    value = os.environ.get(name)
    if not value:
        raise RuntimeError(f"{name} is required")
    return value


def find_github_artifact(repository, run_id, artifact_name, token):
    params = urllib.parse.urlencode({"name": artifact_name, "per_page": "100"})
    path = f"/repos/{repository}/actions/runs/{run_id}/artifacts?{params}"
    data = request_json(
        "GET",
        f"{github_api_base_url()}{path}",
        headers={
            "Accept": "application/vnd.github+json",
            "Authorization": f"Bearer {token}",
            "X-GitHub-Api-Version": "2026-03-10",
        },
    )

    for artifact in data.get("artifacts", []):
        if artifact.get("name") == artifact_name and not artifact.get("expired"):
            archive_url = artifact.get("archive_download_url")
            if archive_url:
                return archive_url

    raise RuntimeError(f"Artifact was not found or is expired: {artifact_name}")


def github_api_base_url():
    return os.environ.get("GITHUB_API_URL", "https://api.github.com").rstrip("/")


def relay_variables():
    raw = os.environ.get("EXTERNAL_CI_VARIABLES_JSON", "{}")
    try:
        parsed = json.loads(raw)
    except json.JSONDecodeError as error:
        raise RuntimeError("EXTERNAL_CI_VARIABLES_JSON must be a JSON object") from error

    if not isinstance(parsed, dict):
        raise RuntimeError("EXTERNAL_CI_VARIABLES_JSON must be a JSON object")

    variables = []
    for key, value in parsed.items():
        add_variable(variables, key, str(value))
    return variables


def add_variable(variables, key, value):
    if not key:
        raise RuntimeError("External CI variable names must be non-empty")
    if any(variable["key"] == key for variable in variables):
        raise RuntimeError(f"Duplicate external CI variable: {key}")
    variables.append({"key": key, "value": value})


def trigger_pipeline(api_base_url, project, ref, token, variables):
    encoded_project = urllib.parse.quote(project, safe="")
    return request_json(
        "POST",
        f"{api_base_url}/projects/{encoded_project}/pipeline",
        headers={
            "Content-Type": "application/json",
            "PRIVATE-TOKEN": token,
        },
        body={
            "ref": ref,
            "variables": variables,
        },
    )


def wait_for_pipeline(api_base_url, project, pipeline_id, token, poll_interval, timeout_seconds):
    encoded_project = urllib.parse.quote(project, safe="")
    deadline = time.monotonic() + timeout_seconds
    last_status = None

    while True:
        data = request_json(
            "GET",
            f"{api_base_url}/projects/{encoded_project}/pipelines/{pipeline_id}",
            headers={"PRIVATE-TOKEN": token},
        )
        status = data.get("status", "unknown")
        if status != last_status:
            print(f"External CI status: {status}")
            last_status = status
        if status in TERMINAL_STATUSES:
            return status
        if time.monotonic() >= deadline:
            raise RuntimeError("Timed out waiting for external CI")
        time.sleep(poll_interval)


def request_json(method, url, headers, body=None):
    data = None
    if body is not None:
        data = json.dumps(body).encode("utf-8")
    request = urllib.request.Request(url, data=data, method=method, headers=headers)
    try:
        with urllib.request.urlopen(request, timeout=60) as response:
            response_body = response.read().decode("utf-8")
    except urllib.error.HTTPError as error:
        raise RuntimeError(f"{method} request failed with HTTP {error.code}") from error
    except urllib.error.URLError as error:
        raise RuntimeError(f"{method} request failed: {error.reason}") from error

    if not response_body:
        return {}
    return json.loads(response_body)


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as error:
        print(str(error), file=sys.stderr)
        raise SystemExit(1)
