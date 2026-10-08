"""driver/records.py

The README's records, from the embedder's side: 0x00 resets, 0x01 carries one
round's input bits, and each input record is answered by one output record.
Used by evaluation and by the cross-program test.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def exchange(process, rounds, m):
    """Reset a running infer process, send one example's rounds, and return the
    m output bits answered for each. A round's bits are a sequence of 0 and 1,
    input wire k at index k, packed LSB-first into the record."""
    raise NotImplementedError
