"""driver/build.py

Experiment file → binary. Every key becomes a #define in build/<name>/config.h
(the program's read-only configuration is compiled in), the word type is set
from execution.word_bits, and make builds the program. For infer, the model
file is compiled in as build/<name>/model.h.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def build(experiment, program, model=None):
    """Build `train`, or `infer` with the given model file compiled in, unless the
    binary is fresh. Return the binary's path."""
    raise NotImplementedError
