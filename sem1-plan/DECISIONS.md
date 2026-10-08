# SYN Phase 1: Decisions

Priority-ordered A/B decisions for `mutant`. `^&` marks a constraint from Prof. Jefferson.
Drafted by DELTA and THREAD. Full text of earlier rounds is in git history (round 1 `43c6dea`, round 2 `6d72d4e`, round 3 `0aa4c47`).

`ARCHITECTURE.md` is the authoritative, self-contained description of the result. This file records the decisions behind it. The Settled sections below have been updated to match the architecture stage (1C), whose own rulings are at the end.

## Round 1 rulings (mutant, 2026-10-04)

| # | Topic | Ruling |
|---|---|---|
| 1 | Variant attachment | **A.** One binary per algorithm; one file per component; `#if` in code. Target figure ^&: one 2D plot, y = loss, x = time, one line per binary (combination of optimisations). |
| 2 | Parallel partition | **A.** Declared per mode. Individual mode at population 1 = one individual, examples serial, ticks serial, instructions parallel. Individual mode adds a component at the Individuals level that updates structure per example. |
| 3 | Skipped-gate wire | **Neither: it's a protocol variation.** Hold (A) is the P1 reference protocol. |
| 4a | Arena boundaries | Superseded by "rounds" (round 2, item 3). |
| 4b | Offspring arena | Algorithm variation. **A** (blank) is the P1 reference. |
| 5 | Task interface | **A.** `dataset.h` + `task.h`. |
| 6a | Encoding defaults | Out of scope. All scoring is task implementation. |
| 6b | `max_ticks` | **A.** Depth ceiling, first-class hyperparameter. |
| 7 | Trace | **A** for P1. Traces must travel up several levels, so backprop probably suits individual mode better. |
| 8 | Address slack | **A.** Runtime scalar, worth trying. |
| 9 | Selector history | A over B, but reopened via the veto (round 2, item 1). |
| 10 | Axes disagreeing | Reopened under "config above the build" (round 2, item 4). |
| — | Stop condition | Algorithm variation. Infrastructure needs room for zero-error and patience. |

**Vetoed / struck from Settled:**
- Selector `scores → indices` / Mutator `genome → genome` (fails backprop).
- Advance mask (implementation detail, out of scope). The idea it served stands: individuals never wait on each other example by example.

**Clarified:** RNG is Run-level. P1 is exactly as intended.

## Settled (round 1, updated by 1C)

- **Hierarchy:** training is Study → Experiment → Run → Generation → Individual → Example → Round → Tick → Instruction. The deployed program is Deployment → Example → Round → Tick → Instruction. Four components own all the loops: **Driver**, **Evolver**, **Harness**, **Kernel** (see 1C). Round loops once when R = 1, so the level is degenerate for XOR/MUX/MNIST but still a level. *(Superseded by the Phase 1 revision, item 1, 5.)*
- **Data rules:**
  1. Config moves down, read-only, written once at Experiment (the experiment file) ^&.
  2. Results move up one level at a time, **unreduced until the Selector**, which owns every reduction over examples.
  3. Buffers are allocated at their owning level and written below it by pointer. The Arena is allocated per individual (once at start-up when deployed), cleared per example and written per tick. *(Superseded by the Phase 1 revision, item 1.)*
- **One level per function.** `main` is the whole pipeline in a few tiered loops.
- **Ready is protocol, not task.** The Kernel reads it. Two independent protocol keys: ready's start value (0 or 1) and the value that means ready (0 or 1). The Harness writes the start value at the start of every round. Reference: start 0, ready on 1 (it makes early training faster); the other three combinations are comparisons.
- **Batching is dataset preparation, not task.** The **Verifier** (train-only) measures correctness only. Cost terms (ticks, live gates, address space) travel up and are weighted at selection. Task is not a hierarchy level.
- **Cost prices live gates, not present gates** (protects the inert reservoir under hold + slack).
- **Examples are a pure lookup** on (seed, generation, position within the generation). Nothing prepares or shuffles a stored order, and *epoch* is not a concept.
- **Determinism under thread count** (same seed, 1 vs N threads, bit-identical) is the test of the declared partition.
- **Vocabulary:** *active set*, *clocked/multi-rate*, *event-driven*, *quiescent*; no name reused across levels; *round* (below).

## Round 2 rulings (mutant, 2026-10-04)

| # | Topic | Ruling |
|---|---|---|
| 1 | Selector / Mutator | **Generations (Selector + Mutator) > Individuals (Trainer).** The within-lifetime structure updater is the **Trainer**, a separate component at Individuals. Evolver vs Selector+Mutator stays tracked, to be settled on contact with implementation. |
| 2 | Variation taxonomy | **A.** Task, execution and replicate are "the behaviour of the world around the model", not of the model. |
| 3 | Rounds | **A.** |
| 4 | Config above build | **B.** Also: hyperparameter sweeps are impractical under A. |
| — | Output encoding | "Of major importance." Becomes round 3 item 1. |
| — | THREAD's scope rule | A strong **hypothesis**. Watch it. |

## Settled (round 2 additions, updated by 1C)

- **Components by level:** Run and Generation are owned by the Evolver, which calls the Selector (cross-individual, never reads a genome) and the Mutator after each generation, and the Exporter at the end of a run. Individual and Example are owned by the Harness, which calls the Trainer (individual mode) between examples. Round has the Kernel (which owns Tick) and, in train, the Verifier. *(Superseded by the Phase 1 revision, item 1, 4, 5.)*
- **Variations:** a behavioural tree, protocol (train and infer must agree) > algorithm (fixed for one execution of train, absent from infer) > parameter (independent within one execution), with parameters scoped to their algorithm. **World axes:** task (varies between Experiments), replicate (seed; varies between Runs; lines are means over seeds). Execution is configured (see the 2B ruling below), and the machine is recorded, never configured. Frontier execution (tick modulo, next-index) is protocol, and each scheduling scheme is an alternative Kernel.
- **Rounds:** a round is one input → ready → output handshake. An example is self-contained, order-independent, always parallelisable, and contains 1..R rounds with declared graded rounds. The arena persists across ticks and rounds and is cleared between examples. Streaming = one long example, chunked for parallelism.
- **Configuration:** one experiment file per experiment is the single source of truth, and its schema is the taxonomy (binding time derived from tree position). Protocol and algorithm choices are compile-time; builds are cached by their hash (`build/<hash>/`). Parameters are runtime and accepted only from an experiment file whose hash matches the binary. The experiment file is embedded in the binary and emitted with every output. A figure is a set of experiment files. Config is a workbench artifact: the deployed program never sees one.

## Hypotheses (tracked, not settled)

- **A component lives at the level where its inputs are scoped** (THREAD).
- **Evolver vs Selector + Mutator:** which factoring survives implementation (mutant).

## Round 3 rulings (mutant, 2026-10-04)

| # | Topic | Ruling |
|---|---|---|
| 1 | Codec | **B.** Encoder and Decoder are separate components. Hypothesis: many encoders may share a decoder. |
| 2 | Scripting interface | **A.** One Python driver. It enables resumable runs, sweeps that skip to the train-execution stage, and automated multi-run pipelines (e.g. for new datasets). |
| 3 | Terminology | **A.** *experiment*, *experiment file*, **Run** (hierarchy level), `infer/` (inference binary). The codebase is an experimental **workbench**. |
| — | THREAD: "binaries must not accept a descriptor path" | **Rejected.** Discipline can't be eliminated, only made the responsibility of code rather than the user. Binaries **may** accept experiment files directly. That flexibility is contained behind the interface and may help unexpectedly, especially for agents. |
| — | THREAD: report carries the world axes | **Accepted.** |

THREAD's restatement (round 4): **one authority, not one capability.** In the experiment path, only the driver reads the experiment file. That path carries the reproducibility guarantee; hand invocations sit outside it, and the hash check contains them.

## Settled (round 3 additions, updated by 1C)

- *Superseded by 2A ruling E:* the Encoder and Decoder, the output layout, and per-layout attribution no longer exist. See the 2A rulings below.
- **Shared implementations:** the Harness and Kernel each have a lane implementation (train) and a packed implementation (infer), pinned by a lane-vs-packed differential test. *(Superseded by the Phase 1 revision, item 7, 8, 10.)*
- **Driver:** a thin Python driver is the experiment's entry point. Its stages are keyed by their inputs:
  1. build
  2. write the Dataset files (Source flattened to bits, labels through the target)
  3. train, once per seed (the Exporter writes each run's model file)
  4. build infer with the model compiled in
  5. evaluate infer on held-out examples, reading answers back through the target and comparing with labels, plus perf/energy
  6. report

  A sweep re-runs stage 2 only. Binaries may also take an experiment file directly; the embedded hash check refuses mismatches.
- **Reports** carry the world axes (thread count, machine, lane width, compiler and flags) next to the experiment file and commit. The experiment file is what was computed; the report is what computed it. Layout: `runs/<hash>/<seed>/`.
- **Vocabulary:** *experiment*, *experiment file*, **Run** (hierarchy level), `infer/` (binary; `run/` is retired), *workbench*.

## 1C rulings: architecture stage (mutant, 2026-10-05)

| Topic | Ruling |
|---|---|
| Data scoping | **Source → Encoder → Dataset**, all at Experiment. The Dataset is read-only below. Run owns only the seed. *(Superseded in 2A: no Encoder or Decoder.)* |
| Example order | A pure lookup on (seed, generation, position). Nothing prepares an order; *epoch* is retired. |
| Grading | The Verifier is train-only and called at Round on graded rounds. The Decoder is at Round in both programs; the Harness tells it which rounds to skip. *(Superseded in 2A: no Encoder or Decoder.)* |
| Reduction | Only the Selector combines per-example results (it's selection policy, and lexicase selection needs them unreduced). Neither the Kernel nor the Harness reduces. |
| Loop ownership | Only components named for an action own loops. A component may own adjacent levels; a loop whose parallel/serial choice varies is declared in exactly one place. The Driver (tooling) is exempt. |
| Loop owners | **Driver** (Study, Experiment), **Evolver** (Run, Generation), **Harness** (Individual, Example; Deployment, Example when deployed), **Kernel** (Round, Tick: owns the tick counter, the ready check and the tick limit). *(Superseded by the Phase 1 revision, item 5.)* |
| Harness | One component in both programs: drives the README protocol (write inputs, run to ready, read outputs). Replaces the Tester/Inferrer split; the Verifier is what it calls to check. |
| Example range | The Harness measures the range of examples it is handed. The split (1 example per piece of work in P1, all of them under the Trainer) is decided in one place, outside it. *(Superseded by the Phase 1 revision, item 1.)* |
| Deployed program | `infer` is the product: Deployment → Example → Round → Tick → Instruction. No Study, Experiment, Run or Individual; no Config, Selector, Mutator, Trainer or Verifier. Its only input is the model file. |
| Model file | Written by the **Evolver** at the end of a run: the best genome plus the Encoder and Decoder ids and settings. Training's only output; the deployed program's only input. *(Superseded in 2A: no Encoder or Decoder.)* |
| Held-out evaluation | The Driver's job, not the product's. Its accuracy is deliberately not the Verifier's error: one drives selection, the other is reported. |
| Label encoding | Moved from the Decoder to the **Encoder** (it alone reads the Source). *(Superseded in 2A: no Encoder or Decoder.)* |
| Tree notation | `( )` lists every component acting at a level (its loop owner and what that owner calls there). `#` lists state created or changed at that level by any configuration. |
| Naming | Invented names must exclude something, and must not name data a component would track. Established field terms win where they exist, so **Run** is kept. Rejected: Tester, Inferrer, Operator, Host, Evaluator, Sequencer, Clock, Runner, Evolution. |
| Documentation | `ARCHITECTURE.md` is self-contained: it assumes only the README and defines every other term before use. |

## 2A rulings: filesystem stage (mutant, 2026-10-07)

| # | Topic | Ruling |
|---|---|---|
| E | Encoding | **No codec.** The model is fed raw data as bits and learns its own encoding. The Encoder, Decoder and output layout are removed. The Driver flattens the Source into bits and chunks it into rounds. The only output convention left is the **target** (label → expected bits, and its inverse for evaluation). Error and attribution are bitwise. |
| F | Example boundaries in infer | One process per example by default; a reset record is the streaming option, not built in P1. *(Superseded by the Phase 1 revision, item 6.)* |
| G | Tick limit in infer | Forced closure: the model always outputs, one output record per input record, with no flag. |
| H | `build/`, `runs/`, `data/` | Git-ignored, but keyed by experiment `name` and meant to be read by developers and agents. |
| — | Model file | Compiled into infer ("one model is one file"): canonical genome, input/output sizes, room for an initial memory state. |
| — | Ready | Two independent protocol keys, start value (0/1) and ready value (0/1), giving four combinations. Reference: start 0, ready on 1 (faster early training). The Harness re-initialises ready at the start of every round (option a). README describes ready generically. |
| — | Ready check | After each tick only, never before the first: every round runs at least one tick, so the start value alone can never answer. |
| — | Layout | Lane code lives in `train/`, packed code in `infer/`. Codecs, if any, never know whether bits are laned or packed. *(Superseded by the Phase 1 revision, item 7, 8.)* |
| — | `config.h` | `#define`s. |
| — | Canonicalisation | Config-driven, done by the Exporter before it writes the model file. |
| — | Exporter, Logger | `canonical` → **Exporter**: canonicalises the best genome and writes the model file, so data leaves a run one way (Evolver → Exporter → model file). `log` → **Logger**: keeps the run log; every component writes its own events. Both are called components, not loop owners. Checkpoints stay with the Evolver: training form, read back only by it. |

The filesystem plan is `FILESYSTEM.md`.

## 2B rulings (mutant, 2026-10-08)

| Topic | Ruling |
|---|---|
| Execution | **Explicit configuration** (`execution.*`: thread count, CPU/GPU backend, lane width, ...), compile-time or start-up as each key needs, so studies can compare runtimes. Defined by changing time and memory, never output. |
| Hashes | The **experiment hash** (protocol, training, inference, parameter, task; not replicate or execution) identifies results, guards `runs/`, and is held fixed by the determinism test. The **build hash** adds compile-time execution keys and identifies a binary. |
| Machine | Processor, OS and compiler version: never configured, always recorded in reports. Runtime comparisons are valid only on one machine. |
| Determinism test | Extends from thread count to every execution key: same experiment hash and seed → identical results. |
| Key prefixes | `protocol.*`, `training.*`, `inference.*`, `parameter.*`, `task.*`, `replicate.*`, `execution.*`. "Algorithm" is the collective term for training + inference, in prose only. *(Revised below: `replicate.*` is replaced by `experiment.seeds`.)* |

The first stubs were retired (`sem1-plan/nand-evolver-old/`, kept for reference only). Reviewing them exposed a flaw in the Phase 1 hierarchy, revised below; 2B restarts from the revision.

## Phase 1 revision (mutant, 2026-10-08)

| # | Topic | Ruling |
|---|---|---|
| 1 | Individual | **No Individual level.** A genome is only "individualised" by memory persisting across rounds, which is what an example already is. The Evolver flattens (genome × example) and splits the work; the work unit is a genome and a range of examples. Supersedes the Hierarchy (round 1), Components by level (round 2) and Example range (1C). |
| 2 | Individual mode | An **Evolver variant** (`#if`): its work unit is a genome's examples in order, and it calls the Trainer between them. |
| 3 | Example | **An example is a lifetime**, in the streaming extension too: memory persists across its rounds and never across examples. An inherited memory state is only what a reset sets. |
| 4 | Harness | **Owns no loop.** Called with arguments, returns a value: at Round it writes the inputs and ready's start value, calls the Kernel and reads the outputs; at the start of an example it resets the Arena to a start state passed in (none = all 0). It alone knows the wire layout. It does not know where inputs come from or where outputs go. |
| 5 | Loop owners | **Driver** (Study, Experiment), **Evolver** (Run, Generation, Example; the Example function in its own file), infer's **main** (Deployment: one loop over records, in which an example is the span between resets rather than a level), **Kernel** (Round, Tick). Supersedes Loop owners (1C). `main` may own loops: the rule exists to stop data owning them, and the program acts. |
| 6 | Records | infer frames its input: header `0x00` = reset, `0x01` = input (then `i` bits). The Arena never sees the header, so every input pattern stays valid and boundaries never depend on timing. A process starts reset. Supersedes F (2A). |
| 7 | Word per wire | Each wire is one `word`, a build-time typedef: `uint8_t` in both programs in P1, `uint64_t` for train in P2. Bit `j` holds example (`j` mod lane width). Keys `execution.word_bits` (8 in P1) and `execution.lane_width` (1 or `word_bits`; 1 in P1). Model files store memory states as packed bits. Supersedes Layout (2A) and Shared implementations (round 3). |
| 8 | One implementation | One Harness and one Kernel, in `core/`, shared by train and infer. `~(a & b)` on whole words is correct at any width, so P2's lanes change only the typedef and how the Harness fills words. |
| 9 | Per-lane results | Ticks and error are per lane from the start (arrays, even at width 1). Finished lanes freeze: `new = (old & done) \| (next & ~done)`. |
| 10 | Tests | `test_protocol` asserts every Arena word is `0` or all ones after every tick (P1). A Python cross-program test runs one multi-round example through train and through infer as `0x01` records, and compares. The lane-vs-packed differential test returns in P2 as a lane-width test. |
| 11 | Config | **Never passed.** Compile-time keys are `#define`s; run-time parameters are one global, read-only after start-up. |
| 12 | Seeds | Not program configuration and **outside the experiment hash**, which covers only what is computed. Where seeds are stated (experiment file, Driver command line or Study) is open. The `replicate.*` prefix is removed. |
| 13 | Naming | Component first (`harness_*`, `kernel_*`); the type distinguishes genome from model; layout never appears in a name. |
| 14 | Logs | Ordered by wall time: only generations have a real order, and no log may ever be out of generation order (mutant, 2B review 2nd iteration). Forbidden key combinations are stated once, in `core/compat.h`, as `#error`. |
| 15 | Expectation | Seq MNIST results wait for P2's lanes. XOR, MUX and MNIST do not. |
