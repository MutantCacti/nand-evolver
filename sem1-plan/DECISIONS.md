# SYN 1B: Decisions

Priority-ordered A/B decisions for `mutant`. `^&` marks a constraint from Prof. Jefferson.
Drafted by DELTA and THREAD. Full text of earlier rounds is in git history (round 1 `43c6dea`, round 2 `6d72d4e`, round 3 `0aa4c47`).

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

## Settled (round 1)

- **Hierarchy:** Run → Generations → Individuals (Genome) → Examples (Trial) → **Rounds (Handshake)** → Ticks (Arena) → Instructions (Kernel). Each component varies the loop below it. Rounds loops once when R = 1, so the level is degenerate for XOR/MUX/MNIST but still a level (see item 3).
- **Data rules:**
  1. Config moves down, read-only, written once at Run ^&.
  2. Results move up one level, reduced at each boundary.
  3. Buffers are allocated at their owning level and written below it by pointer.
- **One level per function.** `main` is the whole pipeline in a few tiered loops.
- **Ready is protocol, not task.** Ready initialises to all-ones (active low).
- **Batching is dataset preparation; Task is scoring.** `score()` measures correctness only. Cost terms (ticks, live gates, address space) travel up and are weighted at selection. Task is not a hierarchy level.
- **Cost prices live gates, not present gates** (protects the inert reservoir under hold + slack).
- **Example source is indexed, not walked.**
- **Determinism under thread count** (same seed, 1 vs N threads, bit-identical) is the test of the declared partition.
- **Vocabulary:** *active set*, *clocked/multi-rate*, *event-driven*, *quiescent*, *scoring*; no name reused across levels; *round* (below).

## Round 2 rulings (mutant, 2026-10-04)

| # | Topic | Ruling |
|---|---|---|
| 1 | Selector / Mutator | **Generations (Selector + Mutator) > Individuals (Trainer).** The within-lifetime structure updater is the **Trainer**, a separate component at Individuals. Evolver vs Selector+Mutator stays tracked, to be settled on contact with implementation. |
| 2 | Variation taxonomy | **A.** Task, execution and replicate are "the behaviour of the world around the model", not of the model. |
| 3 | Rounds | **A.** |
| 4 | Config above build | **B.** Also: hyperparameter sweeps are impractical under A. |
| — | Output encoding | "Of major importance." Becomes round 3 item 1. |
| — | THREAD's scope rule | A strong **hypothesis**. Watch it. |

## Settled (round 2 additions)

- **Components by level:** Generations: Selector (cross-individual, never reads a genome) + Mutator. Individuals: Trainer (individual mode). The Rounds (Handshake) level stands as written in Settled above.
- **Variations:** a behavioural tree, protocol (train and run must agree) > algorithm (independent in train or run) > parameter (independent within one execution), with parameters scoped to their algorithm. **World axes:** task, execution (changes time, never results), replicate (seed; lines are means over seeds). Frontier execution (tick modulo, next-index) is protocol.
- **Rounds:** a round is one input → ready → output handshake. An example is self-contained, always shuffled, always parallelisable, and contains 1..R rounds with declared graded rounds. The arena persists across ticks and rounds and is cleared between examples. Streaming = one long example, chunked for parallelism.
- **Configuration:** one descriptor per experiment is the single source of truth, and its schema is the taxonomy (binding time derived from tree position). Protocol and algorithm choices are compile-time; builds are cached by their hash (`build/<hash>/`). Parameters are runtime and accepted only from a descriptor whose hash matches the binary. The descriptor is embedded in the binary and emitted with every output. A figure is a set of descriptors.

## Hypotheses (tracked, not settled)

- **A component lives at the level where its inputs are scoped** (THREAD).
- **Evolver vs Selector + Mutator:** which factoring survives implementation (mutant).

- **Many encoders share a decoder** (e.g. binary classification) (mutant).

## Round 3 rulings (mutant, 2026-10-04)

| # | Topic | Ruling |
|---|---|---|
| 1 | Codec | **B.** Encoder and Decoder are separate components. Hypothesis: many encoders may share a decoder. |
| 2 | Scripting interface | **A.** One Python driver. It enables resumable runs, sweeps that skip to the train-execution stage, and automated multi-run pipelines (e.g. for new datasets). |
| 3 | Terminology | **A.** *experiment*, *experiment file*, **Run** (hierarchy level), `infer/` (inference binary). The codebase is an experimental **workbench**. |
| — | THREAD: "binaries must not accept a descriptor path" | **Rejected.** Discipline can't be eliminated, only made the responsibility of code rather than the user. Binaries **may** accept experiment files directly. That flexibility is contained behind the interface and may help unexpectedly, especially for agents. |
| — | THREAD: report carries the world axes | **Accepted.** |

THREAD's restatement (round 4): **one authority, not one capability.** In the experiment path, only the driver reads the experiment file. That path carries the reproducibility guarantee; hand invocations sit outside it, and the hash check contains them.

## Settled (round 3 additions)

- **Encoder and Decoder** are separate protocol components shared by train and infer, and many encoders may share a decoder. A task selects them and their params. The exported model pins the genome plus **both** ids and params (the pairing).
  - **Encoder:** world input → input wires. It has no inverse (nothing ever decodes an input). Its params fix `num_inputs` (e.g. MNIST threshold levels).
  - **Decoder:** owns both output directions, `encode_target(label) → expected bits` and `decode(bits) → value`, plus attribution (below). Its params fix `num_outputs`. That pair is the only true inverse, so the round-trip test `decode(encode_target(v)) == v` targets the **Decoder alone**.
- **The decoder side owns error attribution.** It gives each output bit its significance, a discrete gradient at the output boundary that the Trainer and Mutator use as evidence. Each scheme declares its kind:
  - **bit-attributable:** `out ^ expected` alone gives which bits are wrong and which way to move (one-hot, thermometer). Stays in bit space, lane-parallel.
  - **value-attributable:** fixes aren't bit-local (binary, Gray, float). Decodes per lane at the output boundary, once per round, never per tick.
- **Call levels:** in train, inputs and labels are encoded once at load. Attribution runs per round at the output boundary. In infer, encoding and decoding run per round. The kernel never touches either.
- **Shared declarations, two implementations each:** like the kernel, the Encoder and the Decoder each have a lane and a packed implementation. The shared artifacts are two declarations (scheme, params; plus significance and kind for the Decoder), carried by the model file and pinned by a lane-vs-packed differential test over the whole I/O path. They have different hot-path status: the Encoder's lane form runs once at load, the Decoder's at every round boundary.
- **Driver:** a thin Python driver is the experiment's entry point. Its stages are keyed by their inputs:
  1. compile train
  2. execute train (writes the model file + series)
  3. compile infer (genome installed into encoder/decoder)
  4. execute infer + perf/energy
  5. report

  Stages 3–4 run only with a deployment target. A sweep re-runs stage 2 only. Binaries may also take an experiment file directly; the embedded hash check refuses mismatches.
- **Reports** carry the world axes (thread count, machine, lane width, compiler and flags) next to the experiment file and commit. The experiment file is what was computed; the report is what computed it. Layout: `runs/<hash>/<seed>/`.
- **Vocabulary:** *experiment*, *experiment file*, **Run** (hierarchy level), `infer/` (binary; `run/` is retired), *workbench*.
