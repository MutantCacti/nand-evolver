"""driver/dataset.py

Source to Dataset files: raw data flattened into bits and chunked into
rounds, expected bits via targets.py, and a graded flag per round.

One file per split (train, validation, test), written once per experiment,
deterministically from the experiment file and the seed. No split logic exists
in C. Example order within a file is a lookup, never affected by execution
order, so the C side never shuffles.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def write(config, seed, out_dir):
    """Write the train, validation and test Dataset files. Returns their paths."""
    raise NotImplementedError
