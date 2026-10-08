"""driver/targets.py

The target conventions: how a label becomes the expected values of the output
wires, and how output wires are read back as an answer.

The only output-side convention in the project, and the only place it is
stated. Both directions live here so they cannot drift apart: training writes
expected bits with one, and the Driver reads answers back with the same one in
reverse.

Conventions: raw bits, and one wire per possible answer (one-hot).

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def to_bits(label, target, num_outputs):
    """A label to the expected output wire values."""
    raise NotImplementedError


def from_bits(bits, target):
    """Output wire values back to an answer, the exact reverse of to_bits."""
    raise NotImplementedError
