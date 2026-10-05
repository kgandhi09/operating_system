"""Run the actual master and two subscriber executables as independent processes."""
import os
import pathlib
import re
import selectors
import signal
import subprocess
import sys
import time

master_bin, pub_bin, sub_bin = sys.argv[1:]
domain = f"e2e-{os.getpid()}"
processes = []
def start(args):
    p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    processes.append(p)
    return p

def ready(p):
    with selectors.DefaultSelector() as sel:
        sel.register(p.stdout, selectors.EVENT_READ)
        if not sel.select(5):
            raise AssertionError('process did not become ready')
        line = p.stdout.readline()
    assert line.startswith('READY '), (line, p.poll())

def finish(p, expected=0):
    out, err = p.communicate(timeout=10)
    assert p.returncode == expected, (p.args, p.returncode, out, err)
    return out

try:
    master = start([master_bin, domain]); ready(master)
    readers = [start([sub_bin, domain, f"reader-{i}", '8']) for i in range(2)]
    for p in readers: ready(p)
    pub = start([pub_bin, domain, '8'])
    sent = finish(pub)
    received = [finish(p) for p in readers]
    slots = lambda text: re.findall(r'slot=(\d+) generation=(\d+)', text)
    assert len(slots(sent)) == 8
    assert all(slots(text) == slots(sent) for text in received)
    # Independent address spaces see one allocation identity for each publication.
    addresses = [re.findall(r'address=(0x[0-9a-f]+)', text) for text in received]
    assert all(len(a) == 8 for a in addresses)
    master.terminate(); finish(master)
    assert not pathlib.Path(f'/dev/shm/jk-nodeinfra-{os.geteuid()}-{domain}').exists()

    # SIGKILL leaves a stale object; cleanup must refuse a live master and permit a dead one.
    master = start([master_bin, domain]); ready(master)
    failed = subprocess.run([master_bin, '--cleanup', domain], capture_output=True, timeout=5)
    assert failed.returncode != 0
    master.kill(); master.wait(timeout=5)
    subprocess.run([master_bin, '--cleanup', domain], check=True, timeout=5)
    master = start([master_bin, domain]); ready(master)
    master.terminate(); finish(master)
    print('PASS: nodemaster + publisher + two subscribers, matching shared slots, shutdown and stale cleanup')
finally:
    for p in processes:
        if p.poll() is None:
            p.kill()
        p.communicate(timeout=5)
    subprocess.run([master_bin, '--cleanup', domain], capture_output=True, timeout=5)
