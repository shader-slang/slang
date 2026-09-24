# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""Supervise one finite test-server batch using the existing JSON-RPC tool contract."""

import json
import os
from pathlib import Path
import queue
import signal
import subprocess
import threading
import time

MAX_CELLS = 6
MAX_HEADER = 4096
MAX_BODY = 16 * 1024 * 1024


class ProtocolError(ValueError):
    pass


def _unique_object(pairs):
    """Reject ambiguous JSON objects instead of accepting the last duplicate field."""
    value = {}
    for key, item in pairs:
        if key in value:
            raise ProtocolError("duplicate JSON field: " + key)
        value[key] = item
    return value


def _frame(message):
    body = json.dumps(message, separators=(",", ":"), ensure_ascii=True).encode("ascii")
    if len(body) > MAX_BODY:
        raise ProtocolError("request exceeds frame limit")
    return f"Content-Length: {len(body)}\r\n\r\n".encode("ascii") + body


def _read_responses(stream, raw, events):
    """Drain framed stdout independently of requests, retaining even invalid wire bytes."""
    buffer = bytearray()
    length = None
    failed = False
    try:
        while chunk := stream.read(65536):
            raw.write(chunk)
            raw.flush()
            if failed:
                continue
            buffer.extend(chunk)
            try:
                while True:
                    if length is None:
                        end = buffer.find(b"\r\n\r\n")
                        if end < 0:
                            if len(buffer) > MAX_HEADER:
                                raise ProtocolError("response header exceeds limit")
                            break
                        if end > MAX_HEADER:
                            raise ProtocolError("response header exceeds limit")
                        headers = bytes(buffer[:end]).decode("ascii").split("\r\n")
                        fields = {}
                        for header in headers:
                            name, separator, value = header.partition(":")
                            name = name.lower()
                            if not separator or name in fields:
                                raise ProtocolError("invalid or duplicate response header")
                            fields[name] = value.strip()
                        size = fields.get("content-length", "")
                        if not size.isascii() or not size.isdecimal() or len(size) > 9:
                            raise ProtocolError("invalid Content-Length")
                        length = int(size)
                        if length < 1 or length > MAX_BODY:
                            raise ProtocolError("response body exceeds limit")
                        del buffer[:end + 4]
                    if len(buffer) < length:
                        break
                    body = bytes(buffer[:length])
                    del buffer[:length]
                    length = None
                    value = json.loads(body.decode("utf-8"), object_pairs_hook=_unique_object)
                    # With one request in flight, a queue overflow is an unsolicited flood.
                    events.put_nowait(("response", value))
            except (ValueError, UnicodeError, RecursionError, queue.Full) as error:
                failed = True
                buffer.clear()
                try:
                    events.put_nowait(("protocol-error", str(error)))
                except queue.Full:
                    pass
        if not failed:
            events.put_nowait(("protocol-error" if buffer or length is not None else "eof",
                               "truncated response frame" if buffer or length is not None else "stdout closed"))
    except (OSError, queue.Full) as error:
        try:
            events.put_nowait(("protocol-error", str(error)))
        except queue.Full:
            pass


def _validate_response(message, request_id):
    """Require the server's exact response identity and typed ExecutionResult fields."""
    if (not isinstance(message, dict) or message.get("jsonrpc") != "2.0"
            or type(message.get("id")) is not int or message["id"] != request_id
            or "error" in message or set(message) != {"jsonrpc", "id", "result"}):
        raise ProtocolError("unexpected JSON-RPC response identity/type: " + repr(message))
    result = message["result"]
    if not isinstance(result, dict) or set(result) != {
        "stdOut", "stdError", "debugLayer", "result", "returnCode"
    }:
        raise ProtocolError("invalid ExecutionResult fields")
    for key in ("stdOut", "stdError", "debugLayer"):
        if not isinstance(result[key], str):
            raise ProtocolError("invalid ExecutionResult string: " + key)
    for key in ("result", "returnCode"):
        if type(result[key]) is not int or not -(2**31) <= result[key] < 2**31:
            raise ProtocolError("invalid ExecutionResult integer: " + key)
    # TestToolUtil::getReturnCode maps these two core failures specially. Preserve the
    # signed protocol code (CompilationFailed=-1), not the POSIX CLI exit value 255.
    special_codes = {(0x200 << 16) + 6 - (1 << 31): -1,
                     (0x200 << 16) + 7 - (1 << 31): 2}
    expected = special_codes.get(result["result"], 0 if result["result"] >= 0 else 1)
    if result["returnCode"] != expected:
        raise ProtocolError("inconsistent result and returnCode")
    return result


def run_batch(command, requests, directory, environment, timeout, shutdown_timeout=5):
    """Run at most six serial tool requests and wait for a bounded process lifetime.

    Consider three requests where the second server response is truncated. The first remains
    completed, the second records a protocol error, and the third is incomplete. No retry can
    overwrite this evidence. The first request's deadline starts before process creation.
    """
    if not 1 <= len(requests) <= MAX_CELLS or timeout <= 0 or shutdown_timeout <= 0:
        raise ValueError("batch requires 1..6 requests and positive deadlines")
    directory = Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    rows = [{"args": list(args), "status": "incomplete", "return_code": None}
            for args in requests]
    report = {"command": list(command), "cells": rows, "status": "running",
              "stdout_log": str(directory / "stdout.bin"),
              "stderr_log": str(directory / "stderr.log"), "retries": 0}
    events = queue.Queue(maxsize=8)
    exited = queue.Queue(maxsize=1)
    writers = []
    process = None
    reader = waiter = None
    start = time.perf_counter()
    cell_start = start

    def write(message):
        payload = _frame(message)

        def send():
            try:
                remaining = memoryview(payload)
                while remaining:
                    written = process.stdin.write(remaining)
                    if not written:
                        raise OSError("request pipe made no progress")
                    remaining = remaining[written:]
                process.stdin.flush()
            except (OSError, ValueError) as error:
                try:
                    events.put_nowait(("write-error", str(error)))
                except queue.Full:
                    pass

        thread = threading.Thread(target=send, daemon=True)
        writers.append(thread)
        thread.start()

    def wait_exit():
        exited.put(process.wait())

    with open(report["stdout_log"], "wb", buffering=0) as raw, open(report["stderr_log"], "wb") as err:
        try:
            process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                       stderr=err, env=environment, bufsize=0,
                                       start_new_session=os.name != "nt")
            report["pid"] = process.pid
            reader = threading.Thread(target=_read_responses, args=(process.stdout, raw, events),
                                      daemon=True)
            reader.start()
            waiter = threading.Thread(target=wait_exit, daemon=True)
            waiter.start()
            for index, row in enumerate(rows):
                if index:
                    cell_start = time.perf_counter()
                row.update(request_id=index + 1, status="running")
                try:
                    write({"jsonrpc": "2.0", "id": index + 1, "method": "tool",
                           "params": {"toolName": "slangc", "args": row["args"]}})
                    remaining = timeout - (time.perf_counter() - cell_start)
                    if remaining <= 0:
                        raise queue.Empty
                    kind, value = events.get(timeout=remaining)
                    if kind != "response":
                        row.update(status="server-exited" if kind in ("eof", "write-error") else kind,
                                   error=value)
                        break
                    result = _validate_response(value, index + 1)
                    row.update(status="completed", return_code=result["returnCode"],
                               result=result["result"], response=result)
                except queue.Empty:
                    row.update(status="timeout", error="per-cell deadline exceeded")
                    break
                except (OSError, ValueError) as error:
                    row.update(status="protocol-error", error=str(error))
                    break
                finally:
                    row["elapsed_seconds"] = time.perf_counter() - cell_start
            else:
                report["status"] = "completed"
            if report["status"] != "completed":
                report["status"] = "failed"
            else:
                shutdown_start = time.perf_counter()
                write({"jsonrpc": "2.0", "id": len(rows) + 1, "method": "quit"})
                try:
                    report["process_return_code"] = exited.get(timeout=shutdown_timeout)
                    if report["process_return_code"] != 0:
                        report.update(status="shutdown-failed", error="nonzero server exit")
                except queue.Empty:
                    report.update(status="shutdown-timeout", error="server did not exit after quit")
                report["shutdown_seconds"] = time.perf_counter() - shutdown_start
        except OSError as error:
            rows[0].update(status="launch-failed", error=str(error),
                           elapsed_seconds=time.perf_counter() - start)
            report["status"] = "failed"
        finally:
            if process is not None:
                # The leader may already have exited while a child still owns a pipe.
                # Always clean up our POSIX process group, then join the pipe threads.
                if os.name != "nt":
                    try:
                        os.killpg(process.pid, signal.SIGKILL)
                    except ProcessLookupError:
                        pass
                elif process.poll() is None:
                    process.kill()
                try:
                    process.wait(timeout=shutdown_timeout)
                    report["process_return_code"] = process.returncode
                except subprocess.TimeoutExpired:
                    report.update(status="cleanup-failed", cleanup_error="process did not exit")
                for thread in [*writers, reader, waiter]:
                    if thread is not None:
                        thread.join(timeout=shutdown_timeout)
                if any(thread.is_alive() for thread in [*writers, reader, waiter] if thread):
                    report.update(status="cleanup-failed", cleanup_error="pipe thread did not exit")
                else:
                    for stream in (process.stdin, process.stdout):
                        stream.close()
                if report["status"] == "completed":
                    while not events.empty():
                        kind, value = events.get_nowait()
                        if kind != "eof":
                            report.update(status="protocol-error", error="unexpected response after batch: " + repr(value))
            report["elapsed_seconds"] = time.perf_counter() - start
    return report
