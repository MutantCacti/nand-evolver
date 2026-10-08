"""driver/log.py

Reading a run log. Lines are JSON objects in wall-time order, never out of
generation order. Events the Driver does not know are skipped, so components
may log freely.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def read(path, event=None):
    """Yield a run log's events, or only those with the given name."""
    raise NotImplementedError
