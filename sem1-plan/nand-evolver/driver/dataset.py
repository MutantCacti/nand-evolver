"""driver/dataset.py

Source → Dataset files. Each example's raw bits are chunked into task.rounds
rounds of input; each graded round gets its expected output bits from the
target. One file per split (train, validation, test) in data/<name>/, written
once per experiment and never changed. The file format is the one train's
dataset.c maps.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def write(experiment):
    """Write every split's Dataset file unless fresh. Return data/<name>/."""
    raise NotImplementedError
