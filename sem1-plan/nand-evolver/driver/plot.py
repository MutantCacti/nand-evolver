"""driver/plot.py

Finished runs → figures, e.g. error against time with one line per
experiment (the supervisor's target figure). Lines are means over seeds.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def plot(target, kind="loss"):
    """Plot an experiment or a study from the runs gathered for it. Return the figure's path."""
    raise NotImplementedError
