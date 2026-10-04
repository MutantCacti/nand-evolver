# SYN 1B: Decisions

Priority-ordered A/B decisions for `mutant`. `^&` marks a constraint from Prof. Jefferson.
Drafted by DELTA and THREAD. Round 1 text is in git history (`43c6dea`).

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

## Settled (current)

- **Hierarchy:** Run → Generations → Individuals (Genome) → Examples (Trial) → Ticks (Arena) → Instructions (Kernel). Each component varies the loop below it.
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

## Round 2: open items

### 1. Selector / Mutator: split by scope, not by data type
Round 1 split on data type (scores vs genomes), and backprop breaks it. Proposal: split on **scope**.
- **Selector: cross-individual.** It reads all individuals' records (history included, per old item 9) and returns indices. It never reads a genome, and it's the only place individuals are compared.
- **Mutator: within-individual.** It reads **one** individual: the genome **plus that individual's own evidence** (error, per-output error, per-Nand stats, trace). It never reads another individual, so it stays fully parallel.

Why:
- "No edges between individuals" already rules out the only standard two-genome operator (crossover), so every operator we want is within-individual.
- **One interface, two call sites.** Population mode calls `Mutator(genome, evidence)` from Generations between generations. Individual mode calls the same function from Individuals between examples. Blind mutation, confusion matrix and backprop then become algorithm variations of one component that compose with either mode ^&.
- Evidence is another noun with Individual scope (data rule 3). Its type is fixed by the algorithm variation: empty for blind mutation, stats for the confusion matrix, a trace for backprop.

Costs:
- Evidence must live from evaluation until mutation. Across the Generations boundary that's expensive for traces, which is why backprop suits individual mode (evidence consumed immediately, one level up).
- Informed mutators' rates may depend on evidence, so hyperparameters are no longer separated purely by signature.

**Doesn't fit:** any cross-individual operator that needs genomes (novelty/diversity selection, crossover). It would need a third component. None is ruled in.

- **A.** Scope split as above.
- **B.** Monolithic Evolver (selection + mutation) as one swappable component per structure.
- **C.** Strict data-type split; informed mutation only in individual mode.

**Rec: A.** It keeps both components pure in the sense that matters (parallel, no cross-talk) without banning informed mutation in population mode.

### 2. Variation taxonomy
mutant's behavioural axis: **protocol** (train and run must agree) > **algorithm** (independent in train or run) > **parameter** (independent within one execution). Searched for misfits:

- **Frontier execution is protocol.** Tick modulo adds a gene run/ must honour, and next-index changes execution semantics.
- **Execution variation** (misfit): thread count, CPU vs GPU, lane vs packed layout, behaviour-preserving event-driven scheduling, compiler flags. These change **time, never results** (the determinism test proves it). On the target figure they move a line along x without changing its shape, so a figure either holds them fixed or compares only them. The GPU port is one.
- **Task** (misfit): the problem, not the solver. One figure per task.
- **Replicate** (seed): formally a parameter, but statistical in role. Lines are means over seeds.
- **Parameters are scoped to algorithms** (tournament size is meaningless at population 1; slack is meaningless under release). The taxonomy is a **tree**: protocol → algorithm → its parameters.
- **Category migration:** individual mode is an algorithm variation in train, but becomes a protocol variation if it ever runs on-device.

- **A.** Behavioural tree (protocol > algorithm > parameter) plus three orthogonal axes: task, execution, replicate.
- **B.** The three behavioural categories only. Execution, task and seed are treated as parameters.

**Rec: A.** Execution variations otherwise contaminate the time axis of the target figure without changing any loss curve.

### 3. Rounds (replaces "row" and the iid/sequential flag)
A **round** is one input → ready → output handshake. An **example** is self-contained, always shuffled and always parallelisable, and contains 1..R rounds. The arena persists across ticks and rounds and is cleared between examples. XOR/MUX/MNIST have R = 1; Sequential MNIST has R = 28.

- The iid/sequential distinction becomes R = 1 vs R > 1, not a flag.
- An example must declare **which rounds are graded** (Sequential MNIST grades only round 28).
- **Streaming (mutant's open question):**
  - **A.** A stream is one very long example. Parallelising means chunking into examples, and state resets at chunk boundaries.
  - **B.** Persistence across examples, which breaks "self-contained, shuffled, parallel".

  **Rec: A**, consistent with "one example is always self-contained". The chunk length becomes a parameter that bounds the memory horizon the model can learn.

### 4. Configuration above the build
*THREAD to write. Covers single-source descriptor vs build/runtime split, and absorbs round 1's item 10.*

## THREAD
