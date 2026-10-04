# SYN 1B: Decisions

Priority-ordered A/B decisions for `mutant`. `^&` marks a constraint from Prof. Jefferson.
Drafted by DELTA and THREAD. Full text of earlier rounds is in git history (round 1 `43c6dea`, round 2 `6d72d4e`).

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

## Round 3: open items

### 1. Codec (input/output encoding)
mutant: *"Consider encoder/decoder as two separate or one new component shared between train and run. Compilation of a usable model binary then becomes installation of a network's data into the corresponding encoder/decoder."*

- **The Codec is a protocol component, and it splits the old Task.** The Codec maps between world and wires: `encode(input) → input bits`, `encode_target(label) → expected output bits`, `decode(output bits) → value`. A task **selects** a codec (plus codec params). The Task keeps the dataset and the scorer.
- **Codec params are protocol.** MNIST's k threshold levels per pixel set `num_inputs`. The exported model is therefore (genome, codec id, codec params).
- **Training never decodes.** Scoring compares output bits with `encode_target(label)` in bit space, lane-parallel. Decoding per lane would break SIMD, so `decode` runs only in run/ and in reports.
- **Same code, different call level.** In train the codec runs **once, at Run**: the whole dataset and its labels are encoded to bit form at load, and the hot loops never touch it. In run/ it runs **per round**. The train-side placement follows the scope hypothesis (the codec's inputs are dataset-scoped).
- **Installation:** run binary = codec (front and back) + packed kernel + embedded genome blob.

Options:
- **A.** One Codec component, two directions. Encode and decode in one file make inverse consistency a local, testable property (`decode(encode_target(v)) == v`).
- **B.** Separate Encoder and Decoder components (independently swappable, e.g. a thermometer input with one-hot output).

**Rec: A**, with input and output codecs selected independently inside it, so B's flexibility comes back as two parameters rather than two components.

### 2. Scripting interface (the driver)
*THREAD to write. mutant: both build and run consume the descriptor, so discipline isn't solved yet. A "run" = compile train → execute train (writes a model file) → compile run with that file → report, logged. DELTA's lean: one driver is the **only** entry point; binaries embed their hash and refuse a mismatch; Python driver, with C reading only a trivial key=value form it emits; layout `runs/<hash>/<seed>/` with descriptor + commit; run/ measured with rotateai-simulator-style perf/energy for DOER objective 3.*

### 3. Terminology: "run" now means three things
The top hierarchy level (**Run**), the inference binary (`run/`), and mutant's pipeline ("a run"). mutant also asked for a term for the descriptor.

- **A.**
  - *experiment* = the descriptor and its pipeline (one line on the figure, many replicates)
  - *experiment file* = the descriptor
  - **Run** stays the hierarchy level (one training execution, one seed)
  - the inference binary is renamed `infer/`
- **B.** Keep "run" for the pipeline. Rename the hierarchy level (e.g. **Session**) and the binary (`infer/`).

**Rec: A.** "Experiment" is what the target figure plots. Avoid *trial* (taken at Examples).

## THREAD
