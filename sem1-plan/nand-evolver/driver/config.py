"""driver/config.py

The experiment file schema and its hash.

Flat `key = value` with dotted keys, readable by Python and C alike. Seven
prefixes, matching ARCHITECTURE's taxonomy:

    protocol.*      train and infer must agree, or a saved genome changes meaning
    training.*      fixed for train, absent from infer
    inference.*     fixed for infer, absent from train
    parameter.*     numbers, read at start-up
    execution.*     time and memory, never output: threads, backend, lane width
    task.*          which problem, and how its data becomes bits
    replicate.*     the seeds

Two hashes, because two different things need identifying:

    experiment hash   protocol, training, inference, parameter and task keys;
                      not replicate, not execution. Identifies results, is
                      recorded by every run directory, and is what the
                      determinism test holds fixed while execution keys vary.
                      Task belongs in it: two experiments differing only in
                      their task must not share a hash, or the Driver's refusal
                      to combine mismatched runs would not catch mixing them.
    build hash        the experiment hash plus the compile-time execution keys.
                      Identifies one binary, and is what a binary checks.

The machine itself -- CPU model, OS, compiler version -- is never configured and
always recorded, so runtime comparisons are only valid within one machine.

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


def experiment_hash(config):
    """Hash of the keys that change results. Keys runs/ and guards mixing runs."""
    raise NotImplementedError


def build_hash(config):
    """Hash of the keys that change the binary: the experiment hash plus compile-time execution."""
    raise NotImplementedError
