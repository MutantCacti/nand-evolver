"""driver/study.py

A Study: experiments meant to be compared, and the seeds they share.
studies/<name>.cfg names both. Shared seeds make the comparison paired: noise
from seed choice cancels between lines instead of adding to them.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def load(name):
    """Read studies/<name>.cfg: its experiments and its seeds."""
    raise NotImplementedError


def study(name):
    """Run and evaluate every (experiment, seed) pair the study names, skipping
    finished ones. Return the run directories."""
    raise NotImplementedError
