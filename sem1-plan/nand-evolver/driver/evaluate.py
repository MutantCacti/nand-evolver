"""driver/evaluate.py

Held-out evaluation: the Driver's job, not the product's. Builds infer with
one run's model, feeds it a held-out split as records, reads the answers back
through the target and compares them with the labels. Accuracy is reported,
never fed back into training, and is deliberately not the Verifier's error.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def evaluate(name, seed, split="test"):
    """Evaluate one run's model on a split unless fresh. Return its accuracy,
    time and memory, also written into the run directory."""
    raise NotImplementedError
