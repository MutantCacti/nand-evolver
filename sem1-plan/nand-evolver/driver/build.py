"""driver/build.py

Protocol and algorithm choices into -D flags, then make.

Writes `build/<name>/config.h` as #defines and embeds the experiment file in
train. train is given its protocol, training, parameter and execution defines;
infer is given protocol, inference and execution, so the deployed program never
sees a training key. The model file is compiled into infer, so a deployed
binary carries its model.

Writes both hashes into `build/<name>/`: the build hash is what a binary checks
against itself, and the experiment hash is what keys the run directories, so
changing an execution key rebuilds without orphaning the results it produced.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""


def defines(config, program):
    """The #defines for one program ('train' or 'infer') from an experiment file."""
    raise NotImplementedError


def write_hashes(config, build_dir):
    """Write the experiment and build hashes into build/<name>/."""
    raise NotImplementedError


def build_train(config):
    """Write config.h, embed the experiment file, and make train. Returns its path."""
    raise NotImplementedError


def build_infer(config, model_path):
    """Make infer with model_path compiled in. Returns its path."""
    raise NotImplementedError
