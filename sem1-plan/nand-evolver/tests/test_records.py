"""tests/test_records.py

Cross-program: a genome run through train's path and its exported model run
through infer, as records, answer every round identically.

This is where three guarantees meet, none of which holds by construction any
more:

  - **Canonicalisation preserves behaviour.** The genome below has a junior
    Nand losing a collision and an internal wire nothing writes, so the
    Exporter must prune and renumber it. infer runs only the canonical form.
  - **An example is a lifetime in both programs.** train resets at the start
    of each example (the probe's `reset`), infer on a 0x00 record. The
    examples here are multi-round and the genome carries memory between
    rounds, so resetting between rounds, or failing to reset between
    examples, changes an answer.
  - **The record format is as the README states.** One output record per
    input record, a process starts reset, and a broken record is an error.

Created: 2026-10-08
 Author: Maxence Morel Dierckx
"""

import subprocess

import pytest

from driver import build, experiment, paths, records

INPUTS, OUTPUTS, INTERNAL = 2, 1, 3

# Wires: 0 constant, 1-2 inputs, 3 ready, 4 output, 5-7 internal.
GENOME = [
    (1, 2, 5),      # 5 := ~(in0 & in1)
    (5, 6, 4),      # out := ~(w5 & w6), the senior writer of the output
    (6, 6, 6),      # 6 toggles every tick: memory carried across rounds
    (0, 0, 4),      # junior writer of the output: always loses, pruned
    (7, 7, 3),      # ready := ~(w7 & w7); nothing writes 7, so ready on tick 1
]

EXAMPLES = [
    [[1, 1], [1, 0], [0, 1], [0, 0]],
    [[0, 0]],
    [[0, 0], [0, 0], [0, 0]],     # answers 1, 0, 1: only memory tells the rounds apart
]


def packed_hex(bits):
    value = sum(bit << k for k, bit in enumerate(bits))
    return value.to_bytes((len(bits) + 7) // 8, "little").hex()


def unpacked(hex_text, n):
    value = int.from_bytes(bytes.fromhex(hex_text), "little")
    return [(value >> k) & 1 for k in range(n)]


@pytest.fixture
def trained(tiny, probe, workbench):
    """Run GENOME over EXAMPLES through the probe, export it, and build infer
    with the model. Returns (train's answers per example, the infer binary)."""
    name = tiny("records")
    script = [f"shape {INPUTS} {OUTPUTS} {INTERNAL}"]
    script += [f"nand {a} {b} {out}" for a, b, out in GENOME]
    for rounds in EXAMPLES:
        script.append("reset")
        script += [f"round {packed_hex(bits)}" for bits in rounds]
    out_dir = workbench / "probe-run"
    out_dir.mkdir()
    result = subprocess.run([str(probe(name)), str(out_dir)], input="\n".join(script) + "\n",
                            capture_output=True, text=True, check=True)
    lines = iter(result.stdout.split())
    answers = [[unpacked(next(lines), OUTPUTS) for _ in rounds] for rounds in EXAMPLES]
    binary = build.build(experiment.load(name), "infer", model=out_dir / "model")
    return answers, binary


def infer(binary):
    return subprocess.Popen([str(binary)], stdin=subprocess.PIPE, stdout=subprocess.PIPE)


def test_one_process_answers_as_train_did(trained):
    answers, binary = trained
    process = infer(binary)
    for rounds, expected in zip(EXAMPLES, answers):
        assert records.exchange(process, rounds, OUTPUTS) == expected
    process.stdin.close()
    assert process.wait() == 0


def test_a_process_per_example_answers_the_same(trained):
    answers, binary = trained
    for rounds, expected in zip(EXAMPLES, answers):
        process = infer(binary)
        assert records.exchange(process, rounds, OUTPUTS) == expected
        process.stdin.close()
        assert process.wait() == 0


def test_the_examples_depend_on_memory(trained):
    """Guards the test itself: if no answer depended on earlier rounds, the
    lifetime checks above would pass vacuously."""
    answers, _ = trained
    assert answers[2][0] != answers[2][1]


def test_one_output_record_per_input_record(trained):
    _, binary = trained
    stream = b"\x01\x03" + b"\x00" + b"\x01\x00\x01\x01"
    result = subprocess.run([str(binary)], input=stream, capture_output=True, check=True)
    assert len(result.stdout) == 3 * ((OUTPUTS + 7) // 8)


def test_a_broken_record_is_an_error(trained):
    _, binary = trained
    result = subprocess.run([str(binary)], input=b"\x01", capture_output=True)
    assert result.returncode != 0
