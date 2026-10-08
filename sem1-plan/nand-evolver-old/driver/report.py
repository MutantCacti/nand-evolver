"""driver/report.py

Run logs and execution axes into a report.

The run log says what was computed; the execution axes say what computed it
(thread count, machine, word size, compiler flags). A figure needs both, since
execution changes how long a run takes and never what it produces.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def write(run_dirs, out_path):
    """Gather run logs and execution facts into one report."""
    raise NotImplementedError
