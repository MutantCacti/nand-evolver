"""tests/test_determinism.py

Varying an execution key must not change what a run produces.

Execution keys -- thread count, backend, lane width -- are configuration, but
they are the configuration that may change only time and memory. So for one
experiment and one seed, any two runs whose **experiment hash** matches must
produce an identical model file, identical checkpoints and identical
per-generation records, however their execution keys differ.

This is the test of the declared parallel/serial partition. It covers three
invariants at once: no level declared parallel has a hidden dependency, no
random generator is shared, and example lookup is indexed rather than walked.
Logging may differ in arrival order but not in merged content, since per-thread
buffers are merged in canonical order at generation boundaries.

The machine is not an execution key and is not held fixed by anything here: it
is recorded, and runtime comparisons are only valid within one machine.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def test_model_file_identical_across_execution_keys():
    """One experiment hash, differing execution keys: identical model file."""
    raise NotImplementedError


def test_checkpoints_identical_across_execution_keys():
    """One experiment hash, differing execution keys: identical checkpoints."""
    raise NotImplementedError


def test_generation_records_identical_across_execution_keys():
    """One experiment hash, differing execution keys: identical per-generation records."""
    raise NotImplementedError


def test_experiment_hash_ignores_execution_keys():
    """Changing only an execution key leaves the experiment hash unchanged."""
    raise NotImplementedError


def test_build_hash_tracks_execution_keys():
    """Changing a compile-time execution key changes the build hash."""
    raise NotImplementedError
