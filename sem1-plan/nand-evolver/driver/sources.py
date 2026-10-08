"""driver/sources.py

Raw task data. A Source yields examples as raw bits and labels, in its own
fixed order: XOR and MUX are generated truth tables, MNIST is read from data/.
Nothing is encoded: an image is its pixels' bits.

XOR has inputs (a, b) and answers a ^ b. MUX has inputs (select, a, b) and
answers a when select is 0, b when it is 1. Bits are sequences of 0 and 1,
wire k at index k.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def read(source, split):
    """Yield (input bits, label) for every example of one split of a Source."""
    raise NotImplementedError
