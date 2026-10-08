"""driver/sources.py

Readers for raw task data: the XOR and MUX truth tables, and the MNIST
files.

A Source is the task's raw data as it comes, with its labels. Nothing here
invents a representation; flattening into bits is dataset.py's job.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def read(task):
    """Read one task's Source. Returns its raw records and their labels."""
    raise NotImplementedError
