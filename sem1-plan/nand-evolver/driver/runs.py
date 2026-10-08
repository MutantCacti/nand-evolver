"""driver/runs.py

Finished runs, gathered. A run is found by its directory, never by a list
kept elsewhere, so runs made independently (on another day, by another
command) are gathered alike. A run counts only when its recorded experiment
hash matches the experiment file's current one. A run's stamp key is
(experiment hash, seed), so "complete" and "the same experiment" are one
check: execution keys stay out of it, because they never change results.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def directory(name, seed):
    """runs/<name>/<seed>/."""
    raise NotImplementedError


def gather(name):
    """Every complete run of an experiment whose experiment hash matches."""
    raise NotImplementedError
