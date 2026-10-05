"""Exercise real class-based node executables and graceful signal handling."""
import os
import re
import selectors
import signal
import subprocess
import sys

master_bin, publisher_bin, subscriber_bin = sys.argv[1:]
domain = f"nodeapp-e2e-{os.getpid()}"
processes = []


def start(*args):
    process = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    processes.append(process)
    return process


def ready(process):
    with selectors.DefaultSelector() as selector:
        selector.register(process.stdout, selectors.EVENT_READ)
        assert selector.select(5), "node did not become ready"
        assert process.stdout.readline().startswith("READY ")


def finish(process, expected=0):
    out, err = process.communicate(timeout=10)
    assert process.returncode == expected, (process.args, process.returncode, out, err)
    return out, err


try:
    master = start(master_bin, domain)
    ready(master)
    subscriber = start(subscriber_bin, "--domain", domain)
    ready(subscriber)
    publisher = start(publisher_bin, "--domain", domain)
    sent, _ = finish(publisher)
    received, _ = finish(subscriber)
    assert re.findall(r"PUBLISHED (\d+)", sent) == [str(i) for i in range(10)]
    assert re.findall(r"RECEIVED (\d+)", received) == [str(i) for i in range(10)]
    assert received.count("FINALIZED display") == 1

    for stop_signal in (signal.SIGINT, signal.SIGTERM):
        subscriber = start(subscriber_bin, "--domain", domain)
        ready(subscriber)
        subscriber.send_signal(stop_signal)
        out, _ = finish(subscriber)
        assert out.count("FINALIZED display") == 1

    subscriber = start(subscriber_bin, "--domain", domain)
    ready(subscriber)
    master.terminate()
    finish(master)
    out, err = finish(subscriber, 1)
    assert out.count("FINALIZED display") == 1
    assert "nodemaster stopped" in err

    # CLI errors/help must not construct or register a node.
    result = subprocess.run([subscriber_bin, "--help"], capture_output=True, text=True, timeout=5)
    assert result.returncode == 0 and "--domain NAME" in result.stdout
    result = subprocess.run([subscriber_bin, "--domain"], capture_output=True, text=True, timeout=5)
    assert result.returncode != 0 and "invalid arguments" in result.stderr
    print("PASS: class-based processes, ordered callbacks, SIGINT/SIGTERM, master loss, CLI")
finally:
    for process in processes:
        if process.poll() is None:
            process.kill()
        process.communicate(timeout=5)
    subprocess.run([master_bin, "--cleanup", domain], capture_output=True, timeout=5)
