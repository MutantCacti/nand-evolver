"""driver/plot.py

Run logs into figures, for example loss against time with one line per
experiment.

Runs keep full-detail logs, so a figure is recomputed from them rather than
from anything the Driver remembers.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def plot(kind, runs, out_path):
    """Draw one figure of the given kind from these runs."""
    raise NotImplementedError
