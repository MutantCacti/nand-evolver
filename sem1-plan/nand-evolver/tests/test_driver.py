"""tests/test_driver.py

The Driver's own guarantees, through its boundary functions.

Units first (targets, experiment hashes, stamps, the Dataset file, record
framing), which need no binary. Then the Driver end to end on a tiny
experiment: a run without a seed, a study, gathering, evaluation, and the
command line.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""

import io
import struct
from types import SimpleNamespace

import pytest

from driver import __main__, dataset, evaluate, experiment, paths, records, run, runs, stamp, study, targets


# ------------------------------------------------------------------ units

def test_raw_target_is_the_label_bits_lsb_first():
    assert targets.encode("raw", 5, 3) == [1, 0, 1]
    for label in range(8):
        assert targets.decode("raw", targets.encode("raw", label, 3)) == label


def test_onehot_target_sets_one_wire():
    bits = targets.encode("onehot", 3, 10)
    assert bits == [0, 0, 0, 1, 0, 0, 0, 0, 0, 0]
    for label in range(10):
        assert targets.decode("onehot", targets.encode("onehot", label, 10)) == label


def test_experiment_hash_covers_what_is_computed(tiny):
    base = experiment.load(tiny("base")).hash
    assert experiment.load(tiny("threads", execution__threads=1)).hash == base
    assert experiment.load(tiny("words", execution__bits_per_word=64)).hash == base
    assert experiment.load(tiny("population", parameter__population=9)).hash != base
    assert experiment.load(tiny("ready", protocol__ready_start=1)).hash != base
    assert experiment.load(tiny("task", task__source="mux")).hash != base


def test_an_unknown_prefix_is_refused(tiny):
    with pytest.raises(Exception):
        experiment.load(tiny("seeded", experiment__seeds="0, 1"))


def test_a_stamp_is_fresh_only_for_its_key(workbench):
    output = workbench / "output"
    output.mkdir()
    assert not stamp.fresh(output, "a")
    stamp.stamp(output, "a")
    assert stamp.fresh(output, "a")
    assert not stamp.fresh(output, "b")


@pytest.mark.parametrize("source, answer", [
    ("xor", lambda bits: bits[0] ^ bits[1]),
    ("mux", lambda bits: bits[1] if bits[0] == 0 else bits[2]),
])
def test_the_dataset_file_holds_the_task(tiny, source, answer):
    data = dataset.write(experiment.load(tiny(source, task__source=source)))
    raw = (data / "train.ds").read_bytes()
    assert raw[:4] == b"NDS1"
    num_inputs, num_outputs, rounds, num_examples, num_graded = struct.unpack_from("<5I", raw, 4)
    assert (num_outputs, rounds, num_graded) == (1, 1, 1)
    assert raw[24] == 0x01
    assert num_examples > 0
    size = rounds * ((num_inputs + 7) // 8) + num_graded * ((num_outputs + 7) // 8)
    body = raw[25:]
    assert len(body) == num_examples * size
    for e in range(num_examples):
        example = body[e * size:(e + 1) * size]
        bits = [(example[0] >> k) & 1 for k in range(num_inputs)]
        assert example[-1] & 1 == answer(bits)
    for split in ("validation", "test"):
        assert (data / f"{split}.ds").exists()


def test_records_are_framed_as_the_readme_says():
    process = SimpleNamespace(stdin=io.BytesIO(), stdout=io.BytesIO(b"\x01\x02"))
    answers = records.exchange(process, [[1, 0, 1], [0, 1, 1]], 2)
    assert process.stdin.getvalue() == b"\x00\x01\x05\x01\x06"
    assert answers == [[1, 0], [0, 1]]


# ------------------------------------------------------------- end to end

def test_a_run_without_a_seed_draws_and_records_one(tiny):
    name = tiny("unseeded")
    first, second = run.run(name), run.run(name)
    assert first != second
    assert first.parent == second.parent == paths.RUNS / name
    for directory in (first, second):
        assert (directory / "model").exists()
        assert (directory / "log").exists()


def test_a_run_made_again_is_skipped(tiny):
    name = tiny("again")
    directory = run.run(name, 4)
    made = (directory / "model").stat().st_mtime_ns
    assert run.run(name, 4) == directory
    assert (directory / "model").stat().st_mtime_ns == made


def test_gathering_counts_only_the_same_experiment(tiny):
    name = tiny("gathered")
    run.run(name, 1)
    run.run(name, 2)
    assert len(runs.gather(name)) == 2
    tiny("gathered", parameter__population=9)
    assert runs.gather(name) == []


def test_a_study_runs_every_pair_on_shared_seeds(tiny):
    tiny("left")
    tiny("right", task__source="mux")
    (paths.STUDIES / "pair.cfg").write_text("experiments = left, right\nseeds = 0, 1\n")
    directories = study.study("pair")
    assert sorted(str(d.relative_to(paths.RUNS)) for d in directories) == \
        ["left/0", "left/1", "right/0", "right/1"]


def test_evaluation_reports_an_accuracy(tiny):
    name = tiny("evaluated")
    run.run(name, 0)
    result = evaluate.evaluate(name, 0)
    assert 0.0 <= result["accuracy"] <= 1.0


def test_the_command_line_lists(tiny):
    tiny("listed")
    assert __main__.main(["list"]) == 0
