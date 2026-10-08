"""driver/evaluate.py

Held-out evaluation: drives infer over stdin and stdout, reads its answers
back through the task's target, and compares them with the labels.

This comparison is deliberately not the Verifier's. The Verifier produces a
per-example error to drive selection; this produces accuracy for a report.
They answer different questions and may disagree, which is why neither is
derived from the other.

By default one process runs one example, so a process is started per example
and restarting clears the memory space.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def evaluate(infer_binary, dataset_path, target):
    """Accuracy over a held-out split, with time, memory and energy."""
    raise NotImplementedError
