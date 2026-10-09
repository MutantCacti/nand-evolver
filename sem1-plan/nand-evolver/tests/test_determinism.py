"""tests/test_determinism.py

Varying an execution key never changes what a run produces.

Execution keys (thread count, bits per word) may change time and memory only.
So for one experiment hash and one seed, runs that differ only in execution
must produce an identical model file, identical checkpoints and identical
per-generation records. Wall time is the one field allowed to differ, and is
removed before comparing.

This is the test of the declared parallel split: no work unit has a hidden
dependency on another, no random stream is shared, example lookup is indexed
rather than walked, and every logged count is an integer.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""

import pytest

from driver import experiment, log, run

SEED = 3

VARIANTS = {
    "one_thread": {"execution__threads": 1},
    "four_threads": {"execution__threads": 4},
    "wide_words": {"execution__bits_per_word": 64},
}


def outputs(run_dir):
    checkpoints = run_dir / "checkpoints"
    return {
        "model": (run_dir / "model").read_bytes(),
        "checkpoints": sorted((path.relative_to(checkpoints).as_posix(), path.read_bytes())
                              for path in checkpoints.rglob("*") if path.is_file()),
        "generations": [{key: value for key, value in event.items() if key != "time"}
                        for event in log.read(run_dir / "log", event="generation")],
    }


@pytest.fixture
def runs(tiny):
    reference = tiny("reference")
    variants = {label: tiny(label, **keys) for label, keys in VARIANTS.items()}
    return reference, variants


def test_execution_keys_leave_the_experiment_hash_alone(runs):
    """The build hash may differ (word width is compiled in; thread count is
    not), but the experiment hash never does."""
    reference, variants = runs
    for name in variants.values():
        assert experiment.load(name).hash == experiment.load(reference).hash


@pytest.mark.parametrize("label", sorted(VARIANTS))
def test_execution_keys_leave_results_alone(runs, label):
    reference, variants = runs
    expected = outputs(run.run(reference, SEED))
    actual = outputs(run.run(variants[label], SEED))
    assert actual["model"] == expected["model"]
    assert actual["checkpoints"] == expected["checkpoints"]
    assert actual["generations"] == expected["generations"]
    assert len(expected["generations"]) > 0
