# Filesystem Plan

The planned layout of the nand-evolver codebase, derived from `ARCHITECTURE.md`. One file per component; components that train and infer share live in `src/core/`. Comments name the level(s) a file serves, in the terms `ARCHITECTURE.md` defines.

## Target repo state

```shell
nand-evolver/
├── driver/                     # Driver (Study, Experiment). Python tooling, never shipped
│   ├── __main__.py             # `python -m driver <study>`: entry point
│   ├── study.py                # Study: loop the experiment files of one figure
│   ├── experiment.py           # Experiment: run its stages in order, skipping any whose inputs are unchanged
│   ├── config.py               # experiment file schema (protocol → algorithm → parameters; task, execution, replicate) and its hashes
│   ├── build.py                # protocol + algorithm choices → -D flags → make; one binary per hash in build/<hash>/
│   ├── evaluate.py             # held-out evaluation of infer: answers vs labels → accuracy; time, memory, energy
│   └── report.py               # run series + world axes → report and plot
├── studies/                    # one file per Study: the experiment files of one figure
├── experiments/                # experiment files (the Config)
│   ├── xor.toml
│   ├── mux.toml
│   ├── mnist.toml
│   └── seqmnist.toml           # MNIST fed one row per round: 28 rounds, last graded
├── src/
│   ├── core/                   # shared by encode, train and infer: everything the protocol fixes
│   │   ├── word.h              # the word type and lane width
│   │   ├── genome.h            # Genome and Nand; wire layout (constant, input, ready, output, internal)
│   │   ├── genome.c            # create, copy, validate; the only code that knows the wire layout
│   │   ├── arena.h             # Arena: the memory space of one individual
│   │   ├── dataset.h           # Dataset: encoded examples (rounds of input wires, expected wires, graded flags)
│   │   ├── dataset.c           # read-only mapping of a Dataset file; example lookup by (seed, generation, position)
│   │   ├── rng.h
│   │   ├── rng.c               # stateless random streams derived from (seed, level indices)
│   │   ├── model.h
│   │   ├── model.c             # model file: genome + Encoder and Decoder ids and settings; read and write
│   │   ├── source.h            # one raw Source example: what a task's reader yields
│   │   ├── encoder.h
│   │   ├── encoder.c           # Encoder: Source inputs → input wires; labels → expected output wires
│   │   ├── decoder.h           # Decoder: output wires → answer; per-wire attribution; layout kind
│   │   ├── harness.h           # Harness (Individual/Deployment, Example): examples → rounds, protocol driving
│   │   └── kernel.h            # Kernel (Round, Tick): ticks → instructions; ready check; tick limit
│   ├── lane/                   # train layout: one 64-bit word per wire, 64 examples at once
│   │   ├── harness.c
│   │   ├── kernel.c            # one block per Nand scheduling scheme (every tick, clocked, event-driven)
│   │   └── decoder.c
│   ├── packed/                 # deployed layout: one bit per wire, one example
│   │   ├── harness.c
│   │   ├── kernel.c
│   │   └── decoder.c
│   ├── sources/                # one reader per task's raw data, behind source.h
│   │   ├── xor.c
│   │   ├── mux.c
│   │   └── mnist.c             # also serves seqmnist (rows as rounds)
│   ├── encode/
│   │   └── main.c              # Encoder program: Source → Dataset file, once per experiment
│   ├── train/
│   │   ├── main.c              # one Run: Dataset + parameters + seed → Evolver
│   │   ├── evolver.h
│   │   ├── evolver.c           # Evolver (Run, Generation): generations → individuals; the one place examples are split for parallel work; writes the model file
│   │   ├── selector.h
│   │   ├── selector.c          # Selector: compares individuals; owns every reduction over examples
│   │   ├── mutator.h
│   │   ├── mutator.c           # Mutator: parents → children by random changes
│   │   ├── trainer.h
│   │   ├── trainer.c           # Trainer: changes a genome between examples (individual mode only)
│   │   ├── verifier.h
│   │   ├── verifier.c          # Verifier: produced vs expected wires on graded rounds → error
│   │   ├── series.h
│   │   └── series.c            # per-generation results written for the Driver
│   └── infer/
│       └── main.c              # Deployment: load the model file, run the Harness on live input
├── tests/
│   ├── test_protocol.c         # README semantics: constant 0, reserved inputs, ready, tick limit, reverse-order writeback
│   ├── test_layouts.c          # differential: lane vs packed Harness + Kernel + Decoder agree on every example
│   ├── test_codec.c            # each Encoder/Decoder pair: decode(encode_target(v)) == v
│   ├── test_lookup.c           # example lookup is pure: same (seed, generation, position) → same example
│   └── test_determinism.py     # same seed, 1 vs N threads → bit-identical run
├── build/                      # generated, ignored: build/<hash>/{config.h, encode, train, infer}
├── runs/                       # generated, ignored: runs/<hash>/<seed>/ (model file, series, report)
├── data/                       # raw Sources too large for git (MNIST), ignored
├── .gitignore
├── Makefile                    # builds one hash's binaries from its -D flags; `make test`
└── README.md
```

## Notes

- **Configuration flows down.** `build.py` writes `build/<hash>/config.h` from the experiment file's protocol and algorithm choices; every C file reads it and none writes it. Parameters reach `train` at start-up, and are refused unless their experiment file's hash matches the binary's.
- **Which binaries share what.**

  | Binary | Links |
  |---|---|
  | `encode` | core, sources |
  | `train` | core, lane, train |
  | `infer` | core, packed, sources (live input) |
  | tests | core, lane and packed together, for the differential test |

- **Driver stages** (`experiment.py`), each skipped when its inputs are unchanged:
  1. build
  2. encode (Source → Dataset file)
  3. train, once per seed (→ model file + series)
  4. build infer with the model file
  5. evaluate on held-out examples
  6. report

## Decisions this plan needed that ARCHITECTURE.md doesn't settle

1. **The Encoder is its own program, `encode`.** The Driver owns the Experiment level and is Python, but the Encoder is C because infer links it too. A small `encode` binary lets the Driver call it once per experiment and hand every Run the same read-only Dataset file. The alternative is to encode inside `train` at the start of each run (simpler, but once per run instead of once per experiment).
2. **`train` is one Run per invocation.** It follows from the Driver owning the loop over Runs. Seeds run as separate processes, so they can also run in parallel without sharing anything.
3. **Lane and packed are separate files per component, not `#if` blocks.** Round 1 ruled one file per component with `#if` for algorithm variants. Layout, though, is an execution variation, and the differential test must link both layouts into one binary, which `#if` cannot do. Algorithm and protocol variants still use `#if` inside their component's file (e.g. Kernel scheduling schemes inside each `kernel.c`).
4. **Task code lives in three places.** A task's raw data reader is `src/sources/<task>.c`; its encoding and output layout are Encoder and Decoder schemes chosen in the experiment file; what counts as correct is the Verifier. That replaces round 1's `dataset.h` + `task.h`. `dataset.h` survives as the encoded Dataset, and there is no single "task file".
5. **Thread count is a runtime parameter of `train`,** read by `evolver.c`, the only file that splits work.

## From the current tree

The plan is a target state, not a migration. For orientation, today's `src/` maps onto it as follows:
- `core/word.h`: kept, describing only the word type. The two word semantics become the `lane/` and `packed/` directories.
- `core/genome.h`, `train/genome.c`: become `core/genome.{h,c}`. Output indices must respect the reserved inputs.
- `core/arena.h`, `train/arena.c`: `arena.h` stays in core. Allocation moves to the layout that owns it.
- `train/rng.{c,h}`: move to `core/rng.{c,h}`, reduced to stateless stream derivation.
- `train/task.{c,h}`: retired. Its epoch walk and batching contradict the pure example lookup and the Harness owning packing. The XOR/MUX tables move to `sources/`.
