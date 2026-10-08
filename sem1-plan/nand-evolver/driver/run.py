"""driver/run.py

One Run: one experiment, one seed. The unit the Driver is built around; a
study is many of these.

    1. build train
    2. write the Dataset files
    3. record the run: experiment file, experiment hash, machine, seed
    4. call train, which writes the model file, the log and checkpoints

Each step is skipped when fresh, and train resumes from its checkpoints, so
an interrupted run is finished by running it again.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def run(name, seed=None):
    """Run one experiment once. Without a seed, one is drawn from the operating
    system and recorded. Return the run directory."""
    raise NotImplementedError
