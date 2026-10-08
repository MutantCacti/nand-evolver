"""driver/__main__.py

Command line entry: `python -m driver run <experiment>`, `list`, or
`plot --type loss <experiment|study>`.

The Driver is workbench tooling, never shipped. It owns the Study and
Experiment levels and knows nothing about what happens inside a run.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def main(argv=None):
    """Parse argv and dispatch to experiment, study or plot. Returns an exit code."""
    raise NotImplementedError


if __name__ == "__main__":
    raise SystemExit(main())
