# Filesystem Plan

The planned layout of the nand-evolver codebase, derived from `ARCHITECTURE.md`. One file per component. `src/core/` holds what both C programs share. Comments name the level(s) a file serves, in the terms `ARCHITECTURE.md` defines.

**There is no codec** (mutant, ruling E). The model is fed raw data as bits and learns its own encoding. infer is a pure Nand machine whose interface is the README's input and output regions, and the Driver only flattens data into bits and chunks it into rounds.

## Target repo state

```shell
nand-evolver/
├── driver/                     # Driver (Study, Experiment). Python workbench tooling, never shipped; expected to change
│   ├── __main__.py             # `python -m driver run <experiment>` | `list` | `plot --type loss <experiment|study>`
│   ├── experiment.py           # Experiment (main entry): run its stages, skipping any whose outputs already exist
│   ├── study.py                # Study: a named set of experiments, recoverable from runs made independently
│   ├── config.py               # experiment file schema (protocol → algorithm → parameters; task, replicate) and its hash
│   ├── build.py                # protocol + algorithm choices → -D flags → make; embeds the experiment file and the model
│   ├── sources.py              # raw task data readers (XOR/MUX tables, MNIST files)
│   ├── dataset.py              # Source → Dataset files: raw data flattened to bits and chunked into rounds, expected bits from raw labels, graded flags; one file per split (train, validation, test), once per experiment
│   ├── evaluate.py             # held-out evaluation: drives infer over stdin/stdout, decodes, compares with labels → accuracy; time, memory, energy
│   ├── report.py               # run logs + execution axes → report
│   └── plot.py                 # run logs → figures (e.g. loss vs time per experiment)
├── experiments/                # experiment files (the Config); each has a human-readable `name`
│   ├── xor.cfg
│   ├── mux.cfg
│   ├── mnist.cfg
│   └── seqmnist.cfg            # MNIST fed one row per round: 28 rounds, last graded
├── studies/                    # one file per Study: its name and the names of its experiments
├── src/
│   ├── core/                   # shared by train and infer
│   │   ├── word.h              # the word type
│   │   ├── genome.h            # Genome and Nand (training and canonical forms); wire layout (constant, input, ready, output, internal)
│   │   ├── genome.c            # create, copy, validate; the only code that knows the wire layout
│   │   ├── arena.h             # Arena: the memory space of one individual
│   │   ├── model.h
│   │   ├── model.c             # model file: canonical genome, input/output sizes, optional initial memory state
│   │   ├── harness.h           # Harness (Individual/Deployment, Example): examples → rounds; lane_* (train) and packed_* (infer)
│   │   └── kernel.h            # Kernel (Round, Tick): ticks → instructions; ready check; tick limit; lane_* and packed_*
│   ├── train/                  # the search. Lane layout: one 64-bit word per wire, 64 examples at once
│   │   ├── train.h             # boundary functions of the train components, for main.c and the tests
│   │   ├── main.c              # one Run: experiment file + seed (+ thread count) → Evolver
│   │   ├── config.c            # reads the flat experiment-file format; refuses one whose hash differs from the binary's
│   │   ├── dataset.c           # maps the train Dataset file read-only; example lookup from (seed, generation, position)
│   │   ├── rng.c               # stateless random streams derived from (seed, level indices)
│   │   ├── evolver.c           # Evolver (Run, Generation): generations → individuals; the one place work is split; checkpoints; writes the model file
│   │   ├── selector.c          # Selector: compares individuals; owns every reduction over examples
│   │   ├── mutator.c           # Mutator: parents → children by random changes
│   │   ├── trainer.c           # Trainer: changes a genome between examples (individual mode only)
│   │   ├── verifier.c          # Verifier: produced vs expected wires on graded rounds → error
│   │   ├── canonical.c         # canonicalisation (README), driven by config; called by the Evolver before writing a model
│   │   ├── log.c               # every component logs its own events; per-thread buffers, merged in canonical order at generation boundaries
│   │   ├── harness.c           # lane Harness: packing, protocol driving, Verifier/Trainer hooks, graded-round skipping
│   │   └── kernel.c            # lane Kernel: reference scheme (every Nand, every tick)
│   └── infer/                  # the product. Packed layout: one bit per wire, one example, canonical Nands
│       ├── main.c              # Deployment: compiled-in model; per stdin input record, one output record on stdout (always: forced closure); optional reset record clears the Arena
│       ├── harness.c           # packed Harness
│       └── kernel.c            # packed Kernel
├── tests/
│   ├── test_protocol.c         # README semantics: constant 0, reserved inputs, ready, tick limit, reverse-order writeback
│   ├── test_layouts.c          # differential: lane vs packed Harness + Kernel agree on every example (stub hooks)
│   ├── test_canonical.c        # a genome and its canonical form behave identically
│   ├── test_lookup.c           # example lookup is pure: same (seed, generation, position) → same example
│   ├── test_resume.c           # stop at a generation boundary and resume → bit-identical to an uninterrupted run
│   └── test_determinism.py     # same experiment and seed, 1 vs N threads → identical model file, checkpoints and per-generation records
├── build/<name>/               # generated, git-ignored: config.h, the experiment's hash, binaries
├── runs/<name>/<seed>/         # generated, git-ignored: the experiment's hash, run log, checkpoints, model file, report
├── data/                       # raw Sources (MNIST), git-ignored
├── docs/                       # ARCHITECTURE, DECISIONS and FILESYSTEM, moved here when SYN ends
├── .gitignore
├── Makefile                    # builds one experiment's binaries from its -D flags; `make test`
└── README.md                   # the protocol
```

## Notes

- **Names, not hashes, on disk.**
  - Every experiment file has a human-readable `name`. `build/`, `runs/` and `data/` are git-ignored but meant to be read: they are keyed by name, so an agent can read outputs without the Driver.
  - The hash is stored inside `build/<name>/` and checked by the binary; it's never used as a path.
  - Every run directory records its hash too. The Driver refuses to combine runs whose hashes differ under one name, so editing an experiment without renaming it can't silently mix results.
- **Configuration flows down.**
  - **Experiment files** are flat `key = value` with dotted keys (`protocol.kernel = reference`, `algorithm.selector = tournament`, `parameter.population = 256`), readable by Python and C alike.
  - **`build.py`** writes `build/<name>/config.h` as `#define`s from the protocol and algorithm keys, and embeds the experiment file in `train`. infer gets a protocol-only header (e.g. the Kernel scheme), so the deployed program never sees algorithm keys. `core/` headers read those protocol `#define`s.
  - **`config.c`** refuses an experiment file whose hash differs from the binary's.
- **Execution is not configuration.** Thread count, machine and compiler flags are command-line or build facts, outside the hash, recorded in the run log. Only `evolver.c` reads the thread count.
- **The model is one file.** `build.py` compiles the model file into `infer`, so a deployed binary carries its model. The model file holds the canonical genome, its input and output sizes, and room for an initial memory state (a child starting from its parent's memory).
- **infer's interface.** One input record in, one output record out, lock-step. The model always outputs: at the tick limit it emits whatever its output region holds, exactly as training scores it. By default one process runs one example, so restarting clears the Arena; a reset record does the same within one process when an embedder needs it.
- **Ready polarity is a protocol variant.** A genome that learns the task before learning to signal ready is rewarded by being scored at the tick limit. An active-high ready (models must first learn to hold it low) is a `#define` in the protocol header, so both polarities can be compared.
- **Two implementations of one interface.** `harness.h` and `kernel.h` declare `lane_*` (in `train/`) and `packed_*` (in `infer/`) so both link into the differential test. The lane Harness reaches the Verifier and Trainer only through hooks, so tests can stub them.
- **The Driver is stateless; runs keep everything.**
  - `runs/` holds full-detail logs, so scores and plots can be recomputed without re-running.
  - A Study names its experiments, so experiments run independently can later be gathered as one.
- **Driver stages** for one experiment, each skipped when its outputs exist:
  1. build
  2. Dataset file
  3. train, once per seed
  4. build infer with the chosen model
  5. evaluate on held-out examples
  6. report
- **Splits are stated once.** `dataset.py` writes the train, validation and test splits as separate Dataset files, deterministically from the experiment file and seed, so no split logic exists in C. Example order within the train file is a lookup, never affected by execution order.

## Decisions this plan made beyond ARCHITECTURE.md

1. **`train` is one Run per invocation,** following from the Driver owning the loop over Runs.
2. **Lane and packed are files in `train/` and `infer/`,** because train is always lane and infer always packed. Algorithm and protocol variants are `#if` blocks inside their component's file.
3. **Tasks are named in the experiment file:** its source, bit widths, rounds per example and graded rounds are keys. There are no task files and no codecs.
4. **Error and attribution are bitwise.** With no codec, expected outputs are raw label bits, so the Verifier and Trainer need nothing but `out ^ expected`.
5. **Headers only where shared:** `core/*.h` for both programs, and `train/train.h` for train's components and the tests. Private functions are `static`.

## Answers to the review questions

1. **`config.h` values are `#define`s.** Variants are `#if` blocks, and the preprocessor cannot see `constexpr`. Parameters are runtime.
2. **The Evolver pauses and resumes,** at generation boundaries, where the whole population is at rest. Randomness is pure in (seed, level indices), so a checkpoint is just the generation index plus the population's genomes (and memory states if inherited). The Driver decides *whether* to resume. `test_resume` checks that resuming is bit-identical to never stopping.
3. **Documentation:** the README at the root (the protocol), `docs/` for the plan documents once SYN ends, and the existing per-file header comments.
4. **Headers:** only at shared boundaries (decision 5).
5. **Most overloaded:**
   - **`evolver.c`:** both loops, the work split, checkpoints and the model write. Canonicalisation and logging have moved out; the work split could follow if it grows.
   - **The lane Harness:** packing, protocol driving, hooks and graded skipping. That is its job, but it's the densest file.
   - **Smallest:** `word.h`, `arena.h` and `verifier.c` may merge into their callers once written.

## Rulings from the 2A review (mutant)

- **E. No codec.** The model is fed raw data and learns its own encoding: faster, documented by the data format, and it lets us explore whether Nand networks learn to code data themselves.
- **F. Example boundaries:** one process per example by default; a reset record when needed.
- **G. Forced closure:** the model always outputs, with no timeout flag.
- **H. `build/`, `runs/`, `data/`:** git-ignored, but readable by developers and agents.

## Consequence for ARCHITECTURE.md

Ruling E removes the Encoder, the Decoder and the output layout from the architecture. Training's Experiment level becomes "the Driver turns the Source into the Dataset". The Round level loses the Decoder, and its error is bitwise. The deployed tree's Round loses the Encoder and Decoder. ARCHITECTURE.md and DECISIONS.md should be updated before 2B, so the plan and its spec agree.
