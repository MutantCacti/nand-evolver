"""tests/test_resume.py

A run killed at any moment and started again finishes bit-identical to one
that was never interrupted.

train is called directly with the command line the Driver uses, and killed
without warning at arbitrary times, so a checkpoint caught half-written is
part of what is tested: resuming must never read one. The run directory is
train's alone apart from what the Driver records beforehand, so nothing else
is cleaned up between attempts. The log of a resumed run holds each generation
once, as if it had never stopped: events past the checkpoint resumed from are
discarded, not repeated.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""

import random
import signal
import subprocess
import time

from driver import build, dataset, experiment, log, run

SEED = 11
SLOW = {"parameter__population": 32, "parameter__generations": 400}


def train_command(name, out_dir):
    binary = build.build(experiment.load(name), "train")
    data = dataset.write(experiment.load(name))
    return [str(binary), "--seed", str(SEED), "--dataset", str(data / "train.ds"), "--out", str(out_dir)]


def generations(run_dir):
    return [{key: value for key, value in event.items() if key != "time"}
            for event in log.read(run_dir / "log", event="generation")]


def test_killed_and_resumed_is_identical(tiny, workbench):
    uninterrupted = run.run(tiny("whole", **SLOW), SEED)

    out_dir = workbench / "interrupted"
    out_dir.mkdir()
    command = train_command(tiny("broken", **SLOW), out_dir)
    chance = random.Random(SEED)
    kills = 0
    while True:
        process = subprocess.Popen(command)
        try:
            process.wait(timeout=chance.uniform(0.05, 0.5))
            break
        except subprocess.TimeoutExpired:
            process.send_signal(signal.SIGKILL)
            process.wait()
            kills += 1
            time.sleep(0.01)

    assert process.returncode == 0
    assert kills > 0, "the run finished before it could be interrupted; make SLOW slower"
    assert (out_dir / "model").read_bytes() == (uninterrupted / "model").read_bytes()
    assert generations(out_dir) == generations(uninterrupted)
