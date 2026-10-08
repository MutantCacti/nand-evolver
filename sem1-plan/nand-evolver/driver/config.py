"""driver/config.py

The experiment file schema and its hash.

Flat `key = value` with dotted keys, readable by Python and C alike. Six
prefixes, matching ARCHITECTURE's taxonomy:

    protocol.*      train and infer must agree, or a saved genome changes meaning
    training.*      fixed for train, absent from infer
    inference.*     fixed for infer, absent from train
    parameter.*     numbers, read at start-up
    task.*          which problem, and how its data becomes bits
    replicate.*     the seeds

Execution has no prefix: thread count, machine and compiler flags are
command-line or build facts, outside the hash, recorded in the run log.

A name is never written in the file; it is the experiment file's own filename,
so `mux.cfg` is the experiment named 'mux'.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def load(path):
    """Parse an experiment file into a mapping of dotted key to value."""
    raise NotImplementedError


def name_of(path):
    """The experiment's name: its filename without the .cfg suffix."""
    raise NotImplementedError


def hash_of(config):
    """The hash of an experiment file, stored in build/<name>/ and checked by the binary."""
    raise NotImplementedError
