"""driver/paths.py

Where everything lives, relative to the repository root. Every directory the
Driver reads or writes is named here and nowhere else.

    experiments/<name>.cfg      experiment files
    studies/<name>.cfg          study files
    build/<name>/               config.h, model.h, binaries, build stamp
    data/<name>/                Dataset files, one per split
    runs/<name>/<seed>/         one Run: what the Driver records, what train writes

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""

from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
EXPERIMENTS = ROOT / "experiments"
STUDIES = ROOT / "studies"
BUILD = ROOT / "build"
DATA = ROOT / "data"
RUNS = ROOT / "runs"
