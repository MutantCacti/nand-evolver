"""tests/conftest.py

Shared fixtures for the Python tests, which run from the repository root:

    python -m pytest tests

`workbench` gives each test its own experiments/, studies/, build/, data/ and
runs/ under a temporary directory by pointing driver/paths.py at it. Sources
are still built from the real repository. This relies on every driver module
reading `paths.X` when it is called, never copying it at import.

`tiny` writes a fast experiment into that workbench: xor with a small
population and few generations, any key overridable. Experiments that differ
only in execution keys share an experiment hash, which is what most of these
tests turn on.

`probe` builds tests/probe.c for an experiment (`make probe NAME=<name>`,
alongside `make train` and `make infer`) and returns its path.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""

import subprocess

import pytest

from driver import paths

TINY = {
    "protocol.kernel": "reference",
    "protocol.ready_start": "0",
    "protocol.ready_value": "1",
    "training.evolver": "population",
    "training.selector": "tournament",
    "training.mutator": "uniform",
    "training.exporter": "canonical",
    "training.verifier": "hamming",
    "training.trainer": "none",
    "training.inherit_arena": "false",
    "inference.arena": "static",
    "parameter.population": "8",
    "parameter.generations": "6",
    "parameter.tick_limit": "8",
    "parameter.initial_nands": "4",
    "parameter.max_nands": "16",
    "parameter.internal_wires": "16",
    "parameter.mutation_rate": "50",
    "execution.threads": "2",
    "execution.backend": "cpu",
    "execution.word_bits": "8",
    "execution.lane_width": "1",
    "task.source": "xor",
    "task.target": "raw",
    "task.rounds": "1",
    "task.graded": "0",
}


@pytest.fixture
def workbench(tmp_path, monkeypatch):
    for name in ("EXPERIMENTS", "STUDIES", "BUILD", "DATA", "RUNS"):
        directory = tmp_path / name.lower()
        directory.mkdir()
        monkeypatch.setattr(paths, name, directory)
    return tmp_path


@pytest.fixture
def tiny(workbench):
    def write(name, **overrides):
        keys = dict(TINY)
        keys.update({key.replace("__", "."): str(value) for key, value in overrides.items()})
        text = "".join(f"{key} = {value}\n" for key, value in keys.items())
        (paths.EXPERIMENTS / f"{name}.cfg").write_text(text)
        return name
    return write


@pytest.fixture
def probe():
    def build(name):
        subprocess.run(["make", "-C", str(paths.ROOT), "probe", f"NAME={name}",
                        f"BUILD={paths.BUILD}", f"EXPERIMENTS={paths.EXPERIMENTS}"], check=True)
        return paths.BUILD / name / "probe"
    return build
