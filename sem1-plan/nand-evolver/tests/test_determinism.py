"""tests/test_determinism.py

One experiment and one seed, run with 1 thread and with N threads, must produce
an identical model file, identical checkpoints and identical per-generation
records.

This is the test of the declared parallel/serial partition. It covers three
invariants at once: no level declared parallel has a hidden dependency, no
random generator is shared, and example lookup is indexed rather than walked.
Logging may differ in arrival order but not in merged content, since per-thread
buffers are merged in canonical order at generation boundaries.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def test_model_file_identical_across_thread_counts():
    """Same experiment and seed, 1 vs N threads: identical model file."""
    raise NotImplementedError


def test_checkpoints_identical_across_thread_counts():
    """Same experiment and seed, 1 vs N threads: identical checkpoints."""
    raise NotImplementedError


def test_generation_records_identical_across_thread_counts():
    """Same experiment and seed, 1 vs N threads: identical per-generation records."""
    raise NotImplementedError
