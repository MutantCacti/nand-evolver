"""driver/study.py

Study: a named set of experiments meant to be compared, for example the
lines of one plot.

Owns the loop over Experiments. A study only names its experiments, so
experiments run independently can be gathered into one afterwards.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def load(name):
    """Read a study file: its name and the names of its experiments."""
    raise NotImplementedError


def run(name):
    """Run each of the study's experiments that is not already finished."""
    raise NotImplementedError


def experiments(name):
    """The experiment names this study gathers."""
    raise NotImplementedError
