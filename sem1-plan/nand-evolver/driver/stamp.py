"""driver/stamp.py

Skipping finished work. Every output the Driver makes (a build, a Dataset, a
Run, an evaluation) is stamped with a key computed from its inputs. A stage is
skipped when its output's stamp matches the key it would be made from, and
redone otherwise, so a half-finished experiment resumes and a changed
experiment file invalidates exactly what it changes.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def fresh(path, key):
    """Whether the output at path exists and was made from inputs with this key."""
    raise NotImplementedError


def stamp(path, key):
    """Record that the output at path is complete and was made from inputs with this key."""
    raise NotImplementedError
