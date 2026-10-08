"""driver/targets.py

The one output-side convention: how a label becomes the expected values of the
output wires, and how output wires are read back as a label. Each target is
stated once, in both directions (raw: the label's own bits; onehot: one wire
per possible answer).

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def encode(target, label, m):
    """The m expected output bits for a label."""
    raise NotImplementedError


def decode(target, bits):
    """The label that output bits answer."""
    raise NotImplementedError
