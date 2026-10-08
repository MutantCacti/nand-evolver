"""driver/experiment.py

An experiment file, read. The file is flat `key = value` with dotted keys,
one prefix per kind of choice: protocol, training, inference, parameter,
execution, task. Its name is its file name.

The **experiment hash** covers protocol, training, inference, parameter and
task keys: what is computed. The **build hash** adds execution keys: what
computed it. Neither includes a seed.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def load(name):
    """Read experiments/<name>.cfg, refuse unknown prefixes, and return an Experiment
    with `.name`, `.keys` (key → value text), `.hash` (the experiment hash) and
    `.build_hash`."""
    raise NotImplementedError
