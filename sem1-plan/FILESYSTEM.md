# Filesystem Plan

The planned layout of the nand-evolver codebase, derived from `ARCHITECTURE.md`. One file per component. `src/core/` holds what both C programs share. Comments name the level(s) a file serves, in the terms `ARCHITECTURE.md` defines.

**There is no codec** (mutant, ruling E). The model is fed raw data as bits and learns its own encoding. infer is a pure Nand machine whose interface is the README's input and output regions, and the Driver only flattens data into bits and chunks it into rounds.

**There is one Harness and one Kernel.** Both live in `src/core/` and are compiled into both programs. A wire is one `word`, every bit of it the same value, so the same code serves the reference and the lane optimisation that comes later; see "How the memory space is stored" in `ARCHITECTURE.md`.

## Target repo state

```shell
nand-evolver/
├── driver/                     # Driver (Study, Experiment). Python workbench tooling, never shipped; expected to change
│   ├── __main__.py             # `python -m driver run <experiment>` | `list` | `plot --type loss <experiment|study>`
│   ├── experiment.py           # Experiment (main entry): run its stages, skipping any whose outputs already exist
│   ├── study.py                # Study: a named set of experiments, recoverable from runs made independently
│   ├── config.py               # experiment file schema (protocol, training, inference, parameter, execution, task); experiment and build hashes
│   ├── build.py                # protocol + algorithm choices → -D flags → make; sets the word type; embeds the experiment file and the model
│   ├── sources.py              # raw task data readers (XOR/MUX tables, MNIST files)
│   ├── targets.py              # each target convention, both directions: label → expected bits and back (raw bits, one-hot)
│   ├── dataset.py              # Source → Dataset files: raw data flattened to bits and chunked into rounds, expected bits via targets.py, graded flags; one file per split (train, validation, test), once per experiment
│   ├── evaluate.py             # held-out evaluation: drives infer over stdin/stdout as records, maps output bits back through targets.py, compares with labels → accuracy; time, memory, energy
│   ├── report.py               # run logs + execution choices + machine → report
│   └── plot.py                 # run logs → figures (e.g. loss vs time per experiment)
├── experiments/                # experiment files (the Config); each has a human-readable `name`
│   ├── xor.cfg
│   ├── mux.cfg
│   ├── mnist.cfg
│   └── seqmnist.cfg            # MNIST fed one row per round: 28 rounds, last graded
├── studies/                    # one file per Study: its name and the names of its experiments
├── src/
│   ├── core/                   # shared by train and infer
│   │   ├── compat.h            # every forbidden combination of keys, each an #error naming the pair; included by every translation unit
│   │   ├── word.h              # the word type: one per wire, every bit the same value; WORD_BITS; the lane_width rule
│   │   ├── genome.h            # Genome and Nand (training and canonical forms); wire layout (constant, input, ready, output, internal)
│   │   ├── genome.c            # create, copy, validate; the only code that knows the wire layout
│   │   ├── arena.h             # Arena: one memory space. Type only; whoever owns the level above allocates it
│   │   ├── model.h
│   │   ├── model.c             # model file: canonical genome, input/output sizes, optional initial memory state packed as bits
│   │   ├── harness.h           # Harness: reset to the start-of-example state; run one round of the protocol. Owns no loop
│   │   ├── harness.c           # the protocol in one place: write inputs and ready's start value, call the Kernel, read outputs
│   │   ├── kernel.h            # Kernel (Round, Tick): ticks → instructions; ready check; tick limit; ticks per example
│   │   └── kernel.c            # the reference scheme (every Nand, every tick); alternative schedules are #if blocks here
│   ├── train/                  # the search
│   │   ├── train.h             # boundary functions of the train components, for main.c and the tests
│   │   ├── main.c              # one Run: experiment file + seed → Evolver
│   │   ├── config.c            # reads the flat experiment-file format into the read-only global; refuses one whose build hash differs from the binary's
│   │   ├── dataset.c           # maps the train Dataset file read-only; example lookup from (seed, generation, position)
│   │   ├── rng.c               # stateless random streams derived from (seed, level indices)
│   │   ├── evolver.c           # Evolver (Run, Generation): flattens the generation into (genome, example) pairs and splits them; checkpoints; hands the best genome to the Exporter
│   │   ├── example.c           # the Evolver's function at Example: reset, loop the example's rounds, call the Harness and the Verifier. Not a component
│   │   ├── selector.c          # Selector: compares genomes; owns every reduction over examples
│   │   ├── mutator.c           # Mutator: parents → children by random changes
│   │   ├── trainer.c           # Trainer: changes a genome between examples (individual-mode variant only)
│   │   ├── verifier.c          # Verifier: produced vs expected wires on graded rounds → error per example
│   │   ├── exporter.c          # Exporter: canonicalises the best genome (README; config-driven) and writes the model file via core/model.c; called once by the Evolver at the end of a run
│   │   └── logger.c            # Logger: the run log; every component writes its own events; per-thread buffers merged at generation boundaries, wall-time order within a generation
│   └── infer/                  # the product
│       ├── main.c              # Deployment: compiled-in model; loops over records (0x00 reset, 0x01 input), one output record per input record
│       └── records.h           # the record format: the one-byte kind, and the length of an input record
├── tests/
│   ├── test_protocol.c         # README semantics: constant 0, reserved inputs, ready, tick limit, reverse-order writeback, and that every Arena word stays all-zero or all-one
│   ├── test_canonical.c        # a genome and its canonical form behave identically
│   ├── test_lookup.c           # example lookup is pure: same (seed, generation, position) → same example
│   ├── test_resume.c           # stop at a generation boundary and resume → bit-identical to an uninterrupted run
│   ├── test_records.py         # cross-program: one multi-round example through train, and through infer as 0x01 records → same outputs
│   └── test_determinism.py     # same experiment hash and seed, varying every execution key → identical model file, checkpoints and per-generation records
├── build/<name>/               # generated, git-ignored: config.h, the build hash, binaries
├── runs/<name>/<seed>/         # generated, git-ignored: the experiment hash, run log, checkpoints, model file, report
├── data/                       # raw Sources (MNIST), git-ignored
├── docs/                       # ARCHITECTURE, DECISIONS and FILESYSTEM, moved here when SYN ends
├── .gitignore
├── Makefile                    # builds one experiment's binaries from its -D flags; `make test`
└── README.md                   # the protocol, and the record format
```

## Notes

- **Names, not hashes, on disk.**
  - Every experiment file has a human-readable `name`. `build/`, `runs/` and `data/` are git-ignored but meant to be read: they are keyed by name, so an agent can read outputs without the Driver.
  - Two hashes, never used as paths. The **experiment hash** (the `protocol.`, `training.`, `inference.`, `parameter.` and `task.` keys) identifies results: every run directory records it, and the Driver refuses to combine runs whose experiment hashes differ under one name. The **build hash** adds the compile-time execution keys: it is stored in `build/<name>/` and checked by the binary.
  - `execution.` is excluded as a whole prefix, because it must never change results. **The seed is excluded too**, being the one thing that varies between the runs of one experiment: a hash including it would identify a run rather than a result. Where the seed is stated is undecided.
  - Names come directly from the actual file name of the experiment file. For example, the `name` of `mux.cfg` is 'mux'.
- **Configuration flows down, and is never passed.**
  - **Experiment files** are flat `key = value` with dotted keys (`protocol.kernel = reference`, `training.selector = tournament`, `inference.arena = static`, `parameter.population = 256`, `task.rounds = 28`, `execution.word_bits = 8`), readable by Python and C alike.
  - **`build.py`** writes `#define`s per binary: protocol keys go to both, training keys to `train` only, inference keys to `infer` only, and compile-time execution keys (the word type, the lane width) to whichever binary they shape. It embeds the experiment file in `train`. `core/` headers read the protocol `#define`s.
  - **`config.c`** refuses an experiment file whose build hash differs from the binary's, and fills **one read-only global**. No function takes a config argument: configuration that cannot change and is identical everywhere is not a parameter.
- **Execution is configuration, outside the experiment hash.** `execution.*` keys are explicit so studies can compare runtimes. Varying them must never change results, which `test_determinism` checks. The **machine** (processor, OS, compiler version) is not configured; every report records it.
- **The model is one file.** `build.py` compiles the model file into `infer`, so a deployed binary carries its model. The model file holds the canonical genome, its input and output sizes, and room for an initial memory state, stored as packed bits and unpacked on load so the file does not depend on the word type.
- **infer's interface is records.** `main` owns the loop: `0x01` plus the input region's values is one round, `0x00` is a reset, and one output record leaves per input record. The kind is a byte outside the input values, so every pattern of values stays a valid input, and a known record length lets a short read mean "read more" instead of a signal. An example is the span between resets; starting a process is itself a reset, so one process per example sends no reset records at all. The model always outputs: at the tick limit it emits whatever its output region holds, exactly as training scores it.
- **Ready is two protocol keys.** `protocol.ready_start` (the value the Harness writes into the ready wire at the start of every round) and `protocol.ready_value` (the value that means ready), each 0 or 1, independent, compiled into both programs. The Kernel checks ready after each tick only, so every round runs at least one tick. The reference is start 0, ready on 1: a genome isn't ready until a Nand drives the wire high, which a Nand reading cleared wires does at once, so early training is faster. The other three combinations are lines on the same figure.
- **Results are per example, never per group.** `verifier_verify` writes an error per example and the Kernel reports ticks per example, as arrays. At the reference width each array has one entry; when one word later holds many examples the same code is already right, and the number of examples per word cannot change a result.
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
2. **The Harness and the Kernel are one implementation each, in `core/`.** They are shared rather than mirrored, so the two programs cannot drift apart. Protocol and scheduling variants are `#if` blocks inside their file; the word type and the lane width are build-time choices that the same code already serves. There is therefore no lane-versus-packed differential test in this tree: with one implementation there is nothing to compare it against, and a differential test between two implementations written from one spec by the same hands was never independent anyway. It returns when the lane optimisation does, as a test of one width against another over the same Kernel.
3. **Tasks are named in the experiment file:** its source, bit order, target convention, rounds per example and graded rounds are keys. There are no task files and no input codecs. The only output-side convention is how a label becomes expected bits, stated once in `targets.py` (both directions).
4. **Error and attribution are bitwise.** Expected outputs are bits, so the Verifier and Trainer need nothing but `out ^ expected`. The limit: for a numeric target written in binary, Hamming distance counts a wrong high bit the same as a wrong low bit, so numeric targets need a non-Hamming error. That's outside P1's tasks (XOR, MUX, one-hot MNIST).
5. **Headers only where shared:** `core/*.h` for both programs, and `train/train.h` for train's components and the tests. Private functions are `static`.
6. **`src/train/example.c` is a file of the Evolver, not a component.** The Evolver owns Run, Generation and Example, and "one level, one function" makes Example its own function; it gets its own file because `evolver.c` already carries two loops, the work split and checkpoints.

## Answers to the review questions

1. **`config.h` values are `#define`s.** Variants are `#if` blocks, and the preprocessor cannot see `constexpr`. Parameters are runtime, in one read-only global.
2. **The Evolver pauses and resumes,** at generation boundaries, where the whole population is at rest. Randomness is pure in (seed, level indices), so a checkpoint is just the generation index plus the population's genomes (and memory states if inherited). The Driver decides *whether* to resume. `test_resume` checks that resuming is bit-identical to never stopping.
3. **Documentation:** the README at the root (the protocol and the record format), `docs/` for the plan documents once SYN ends, and the existing per-file header comments.
4. **Headers:** only at shared boundaries (decision 5).
5. **Most overloaded:**
   - **`evolver.c`:** both loops, the work split and checkpoints. Canonicalisation and the model write (Exporter), logging (Logger) and the Example level (`example.c`) have moved out; the work split could follow if it grows.
   - **`core/harness.c`:** the whole protocol, for both programs. It owns no loop, which keeps it short, but it is the file where a mistake is most expensive.
   - **Smallest:** `word.h`, `arena.h`, `records.h` and `verifier.c` may merge into their callers once written.

## Rulings from the 2A review (mutant)

- **E. No codec.** The model is fed raw data and learns its own encoding: faster, documented by the data format, and it lets us explore whether Nand networks learn to code data themselves.
- **F. Example boundaries:** one process per example by default; a reset record when needed, now defined as `0x00`.
- **G. Forced closure:** the model always outputs, with no timeout flag.
- **H. `build/`, `runs/`, `data/`:** git-ignored, but readable by developers and agents.

## Rulings from the 2B review (mutant)

- **Log order is wall time, never out of generation order.** Only generations have a real order; an index within one is arbitrary and may be produced in parallel. `logger.c` merges per-thread buffers at generation boundaries, which is the rest point that makes the guarantee hold. The log is not among the things `test_determinism` compares.
- **Forbidden key combinations live in `core/compat.h`,** one `#error` per incompatible pair, included by every translation unit. No file guards its own combinations.
