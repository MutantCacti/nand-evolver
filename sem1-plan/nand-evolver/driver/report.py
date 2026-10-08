"""driver/report.py

Finished runs → a report: per experiment, the mean and spread over seeds of
error, accuracy, ticks and runtime, next to the execution keys and the
machine that produced them.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def report(target):
    """Report on an experiment or a study from the runs gathered for it. Return the report's path."""
    raise NotImplementedError
