"""driver/runs.py

Finished runs, gathered. A run is found by its directory, never by a list
kept elsewhere, so runs made independently (on another day, by another
command) are gathered alike. A run counts only when its recorded experiment
hash matches the experiment file's current one.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def directory(name, seed):
    """runs/<name>/<seed>/."""
    raise NotImplementedError


def gather(name):
    """Every complete run of an experiment whose experiment hash matches."""
    raise NotImplementedError
