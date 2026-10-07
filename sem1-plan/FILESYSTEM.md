# Filesystem Plan

The planned layout of the nand-evolver codebase, derived from `ARCHITECTURE.md`. One file per component. `src/core/` holds what two or more binaries share. Comments name the level(s) a file serves, in the terms `ARCHITECTURE.md` defines.

## Target repo state

```shell
nand-evolver/
├── driver/                     # Driver (Study, Experiment). Python tooling, never shipped
│   ├── __main__.py             # `python -m driver <study>`: entry point
│   ├── study.py                # Study: loop the experiment files of one figure
│   ├── experiment.py           # Experiment: run its stages in order, skipping any whose inputs are unchanged
│   ├── config.py               # experiment file schema (protocol → algorithm → parameters; task, replicate) and its hashes
│   ├── build.py                # protocol + algorithm choices → -D flags → make; one build per hash in build/<hash>/, experiment file embedded
│   ├── evaluate.py             # held-out evaluation of infer: answers vs labels → accuracy; time, memory, energy
│   └── report.py               # run series + execution axes → report and plot
├── studies/                    # one file per Study: the experiment files of one figure
├── experiments/                # experiment files (the Config)
│   ├── xor.cfg
│   ├── mux.cfg
│   ├── mnist.cfg
│   └── seqmnist.cfg            # MNIST fed one row per round: 28 rounds, last graded
├── src/
│   ├── core/                   # shared by two or more binaries
│   │   ├── word.h              # the word type
│   │   ├── genome.h            # Genome and Nand (training and canonical forms); wire layout (constant, input, ready, output, internal)
│   │   ├── genome.c            # create, copy, validate, canonicalise; the only code that knows the wire layout
│   │   ├── arena.h             # Arena: the memory space of one individual
│   │   ├── dataset.h
│   │   ├── dataset.c           # Dataset file format: encoded examples (rounds of input wires, expected wires, graded flags). encode writes it, train maps it read-only
│   │   ├── config.h
│   │   ├── config.c            # reads the flat experiment-file format; checks its hash against the binary's
│   │   ├── model.h
│   │   ├── model.c             # model file: canonical genome + Encoder and Decoder ids and settings; read and write
│   │   ├── layout.h            # output layout declaration: wire count, value ↔ wires, attribution kind
│   │   ├── layouts.c           # each output layout stated once (one-hot, thermometer, binary, ...)
│   │   ├── encoder.h
│   │   ├── encoder.c           # Encoder: world inputs → input wires; labels → expected wires via the layout
│   │   ├── decoder.h           # Decoder: output wires → answer via the layout; per-wire attribution; lane_* and packed_*
│   │   ├── harness.h           # Harness (Individual/Deployment, Example): examples → rounds; lane_* and packed_* entry points
│   │   └── kernel.h            # Kernel (Round, Tick): ticks → instructions; ready check; tick limit; lane_* and packed_*
│   ├── lane/                   # train layout: one 64-bit word per wire, 64 examples at once
│   │   ├── harness.c           # calls Verifier and Trainer through hooks it is handed, so lane/ never depends on train/
│   │   ├── kernel.c            # reference scheme (every Nand, every tick); alternative Kernels go here
│   │   └── decoder.c           # see open decision C
│   ├── packed/                 # deployed layout: one bit per wire, one example, canonical Nands
│   │   ├── harness.c
│   │   ├── kernel.c
│   │   └── decoder.c
│   ├── sources/                # workbench readers of each task's raw data; linked into encode only
│   │   ├── source.h
│   │   ├── xor.c
│   │   ├── mux.c
│   │   └── mnist.c             # also serves seqmnist (rows as rounds)
│   ├── encode/
│   │   └── main.c              # Encoder program: Source → Dataset file, once per experiment
│   ├── train/
│   │   ├── main.c              # one Run: Dataset + parameters + seed (+ thread count) → Evolver
│   │   ├── evolver.h
│   │   ├── evolver.c           # Evolver (Run, Generation): generations → individuals; the one place work is split; canonicalises and writes the model file
│   │   ├── lookup.h
│   │   ├── lookup.c            # example lookup by (seed, generation, position)
│   │   ├── rng.h
│   │   ├── rng.c               # stateless random streams derived from (seed, level indices)
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
│       └── main.c              # Deployment: the model file, then the Harness on live input (see open decisions A, B)
├── tests/
│   ├── test_protocol.c         # README semantics: constant 0, reserved inputs, ready, tick limit, reverse-order writeback
│   ├── test_layouts.c          # differential: lane vs packed Harness + Kernel + Decoder agree on every example (stub hooks)
│   ├── test_codec.c            # each output layout: decode(encode_target(v)) == v
│   ├── test_canonical.c        # a genome and its canonical form behave identically
│   ├── test_lookup.c           # example lookup is pure: same (seed, generation, position) → same example
│   └── test_determinism.py     # same experiment file and seed, 1 vs N threads → bit-identical run
├── build/                      # generated, ignored: build/<hash>/{config.h, encode, train, infer}
├── runs/                       # generated, ignored: runs/<hash>/<seed>/ (model file, series, report)
├── data/                       # raw Sources too large for git (MNIST), ignored
├── .gitignore
├── Makefile                    # builds one hash's binaries from its -D flags; `make test`
└── README.md
```

## Notes

- **Configuration flows down.**
  - **Experiment files** use a flat `key = value` format with dotted keys (`protocol.kernel = reference`, `algorithm.selector = tournament`, `parameter.population = 256`). The Driver and the C binaries can both read it without a parser dependency.
  - **`build.py`** writes `build/<hash>/config.h` from the protocol and algorithm keys, and embeds the experiment file in each binary.
  - **Parameters** reach `train` at start-up through `config.c`, which refuses an experiment file whose hash does not match the binary's.
- **Execution is not configuration.** Thread count, machine and compiler flags are command-line or build facts, outside the hash. `train` records them in its output, and only `evolver.c` reads the thread count. That is what lets the determinism test compare 1 vs N threads on the same binary.
- **Two implementations of one interface.** `harness.h`, `kernel.h` and `decoder.h` declare `lane_*` and `packed_*` entry points, so both layouts can link into the differential test. The lane Harness reaches the Verifier and Trainer only through hooks passed in by `train`, and tests pass stubs.
- **Canonicalisation** (README) is `genome.c`'s, called by the Evolver before it writes the model file. The packed Kernel reads canonical two-index Nands. This is also a gap in ARCHITECTURE.md, which should say it.
- **Which binaries link what.**

  | Binary | Links |
  |---|---|
  | `encode` | core, sources |
  | `train` | core, lane, train |
  | `infer` | core except `config.c` and `dataset.c` (no Config, no Dataset), packed, infer |
  | tests | core, lane and packed together, train (selected files) |

- **Driver stages** (`experiment.py`), each skipped when its inputs are unchanged:
  1. build
  2. encode (Source → Dataset file)
  3. train, once per seed (→ model file + series)
  4. prepare infer for each model file (see A)
  5. evaluate on held-out examples
  6. report

## Decisions this plan made beyond ARCHITECTURE.md

1. **The Encoder is its own program, `encode`.** The Driver owns the Experiment level and is Python, while the Encoder is C because infer links it too. `encode` lets the Driver run it once per experiment and give every Run the same read-only Dataset file. Encoding inside each run would break the Dataset's Experiment scope.
2. **`train` is one Run per invocation,** following from the Driver owning the loop over Runs.
3. **Lane and packed are separate files.** Layout is not a variation: train is always lane and infer always packed. Round 1's `#if` ruling covers algorithm and protocol variants, which still use `#if` inside their component's file.
4. **Task code lives in three places:** a raw data reader in `sources/`, an encoding and output layout chosen in the experiment file, and what counts as correct in the Verifier. Each output layout is stated once, in `layouts.c`. That answers ARCHITECTURE's open question about the shared output layout with "yes".
5. **Only the reference Kernel is planned.** Alternative Kernels get a place, not files, until there is a run to compare them against.

## Open decisions for mutant

- **A. Model file: compiled into infer, or loaded at start-up?** The README's deployment section (genome read-only, flashable ROM) points to compiled in: `build.py` emits the model as a generated source and `infer` never links the read path. Loading at start-up is simpler for the Driver's evaluation stage.
- **B. What form does infer's live input take?** ARCHITECTURE says the deployed Encoder runs "on live input" but never says what that input looks like. Proposal: one documented record format on stdin. `evaluate.py` produces it from held-out Source data, so `sources/` stays out of the product.
- **C. Does the Decoder know the layout?**
  - **Yes** (as drawn): a lane Decoder attributes 64 examples at once for bit-attributable layouts. ARCHITECTURE's "packing is a property of the Harness and Kernel" then needs the Decoder added.
  - **No:** the Harness unpacks lane words, and one plain Decoder lives in core. That's simpler, but train's attribution becomes per example.
