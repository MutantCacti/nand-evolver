"""driver/__main__.py

The Driver's command line. Every command names an experiment or a study by
its file name, never by path or hash.

    python -m driver run <experiment> [--seed N]     one Run; a seed is drawn and recorded when absent
    python -m driver evaluate <experiment> --seed N  held-out accuracy of one Run's model
    python -m driver study <study>                   every (experiment, seed) pair the study names
    python -m driver report <experiment|study>       a report from finished runs
    python -m driver plot <experiment|study> [--type loss]
    python -m driver list                            experiments, studies and their runs

`main` is importable, for the tests; the module runs it only when executed.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def main(argv=None):
    """Parse the command line, call the one function the command names, and return the exit code."""
    raise NotImplementedError
