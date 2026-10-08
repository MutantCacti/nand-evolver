"""driver/sources.py

Raw task data. A Source yields examples as raw bits and labels, in its own
fixed order: XOR and MUX are generated truth tables, MNIST is read from data/.
Nothing is encoded: an image is its pixels' bits.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def read(source, split):
    """Yield (input bits, label) for every example of one split of a Source."""
    raise NotImplementedError
