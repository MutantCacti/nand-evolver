"""driver/build.py

Experiment file → binary. Compile-time keys (protocol, training, inference,
and execution word_bits, lane_width and backend) become #defines in
build/<name>/config.h, which also sets the word type. The whole experiment
file is embedded in train, which reads its parameters and thread count from
that copy at start-up, so one build serves a sweep over parameters. Task keys
never reach C: train learns the task's shape from the Dataset file. For
infer, the model file is read here and emitted as build/<name>/model_data.h:
its Nands expanded back to (a, b, out), so no C code parses a model file.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def build(experiment, program, model=None):
    """Build `train`, or `infer` with the given model file compiled in, unless the
    binary is fresh. Return the binary's path."""
    raise NotImplementedError
