"""driver/experiment.py

Experiment: one fully specified search, described by one experiment file.

Owns the loop over Runs. Stages are skipped when their outputs already exist,
which is what makes a half-finished experiment resumable:

    1. build train
    2. write the Dataset files
    3. train, once per seed
    4. build infer with the chosen model
    5. evaluate on held-out examples
    6. report

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def run(name, seeds=None):
    """Run every unfinished stage of one experiment. Returns its run directories."""
    raise NotImplementedError


def stages(name):
    """Each stage of this experiment and whether its outputs already exist."""
    raise NotImplementedError
