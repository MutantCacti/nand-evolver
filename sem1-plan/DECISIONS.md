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

mutant: *"Splitting runtime parameter variation and compile-time algorithm variation is less
preferable than a single source of truth. Ideally a run is completely reproducible from a single
top-level configuration/descriptor file. This maybe means placing the configuration above the
build process rather than as a runtime artifact."*

Endorsed, and it makes round 1's item 10 **unnecessary rather than answered**. One descriptor per
experiment is the single source of truth. Tooling reads it, derives the `-D` set, builds or reuses
the matching binary, and runs it. The build becomes a **derived artifact of the descriptor**, so
"a config names a variant the binary wasn't compiled with" cannot occur — the config is what
caused the compilation. Same move as the rest of Settled: structural, not disciplinary.

**The descriptor's schema is the item 2 taxonomy.** Behavioural tree nested as
protocol → algorithm → parameters, with task, execution and replicate as separate top-level
fields. Binding time is then **derived** from where a key sits in the tree, never authored.

The open question is only how far down the tree compilation reaches.

- **A.** Everything compile-time, parameters included (descriptor → generated header). Maximally
  single-source; constants fold; the binary *is* the experiment.
- **B.** Protocol and algorithm compile-time; parameters passed at runtime but only via a
  descriptor whose protocol/algorithm hash matches the binary's embedded hash, so a mismatch is
  refused rather than silently honoured.

**Rec: B**, for a reason that comes from the target figure rather than from convenience.

The x-axis is **time**. Under A, changing one parameter recompiles, so constant folding differs
and per-generation wall time shifts for reasons that are nothing to do with the parameter. A
parameter variation would therefore behave as an **execution variation** (item 2: changes time,
not results), which item 2 says must be held fixed *within* a figure. So A structurally
contaminates the axis the project is graded on, and sweeps (P2c) would also cost one build per
point.

Keeping it honest: if some constant's folding turns out to genuinely matter for kernel speed, the
answer is to **reclassify it as protocol or execution**, not to smuggle it in as a compile-time
parameter. The taxonomy should move, not the binding rule.

Mechanics, assuming B:
- **Cache builds by the hash of the protocol/algorithm choices** (`build/<hash>/`), so a sweep
  that touches only parameters reuses one binary.
- **Embed the descriptor in the binary and emit it with every run's output**, so reproducibility
  is exactly `(descriptor, commit)`.
- **A figure is a set of descriptors**, one per line. That makes mutant's 2D plot a direct
  artifact of the experiment definition rather than something assembled by hand afterwards.

## THREAD

Round 2 review. Item 4 written above. Agreed with items 1–3 as drafted; DELTA's scope-based split
(item 1) is a better cut than the data-type split I was defending, and subsumes it. One edit, one
addition each to items 1 and 2, one general rule.

### Edit made

- **Settled hierarchy** now contains the **Rounds (Handshake)** level. Item 3 establishes that an
  example contains 1..R rounds, so by "one level per function" there is a loop — and therefore a
  function — between Examples and Ticks. The level is degenerate at R = 1, not absent. Flagging it
  because it changes the hierarchy mutant drew, and so changes Phase 2A's file layout.

### Addition to item 1: recompute softens the stated cost

Item 1's honest cost is that evidence must survive from evaluation to mutation, and that traces
across the Generations boundary are expensive. Ruling **7A (recompute)** weakens this: the kernel
is deterministic and the example source is indexed, so a Generations-level informed mutator can
**regenerate** a selected parent's trace by re-running it, rather than retaining `T×W` words per
individual through selection.

So informed mutation in **population** mode stays available at the cost of one extra evaluation
per selected parent, and the default build retains nothing. Individual mode remains the better
fit — evidence is consumed immediately, one level up — but it is a preference, not a restriction.

### Addition to item 2: a misfit on the other axis

The listed misfits (execution, task, replicate) are all **orthogonal axes**. There is also a
variation that sits inside the behavioural tree but breaks the **binding time** the tree implies:

- **Output encoding** (one-hot / thermometer / Gray / raw). Train and run must agree or a deployed
  genome's output bits are meaningless, so it is **protocol**. But it varies with the *task*,
  chosen at **runtime**, not with the binary.

Consequence, and I don't think item 6a's out-of-scope call covers it, since this is deployment
rather than library defaults: **the exported model file must carry its output encoding.**
Otherwise a canonicalised genome is uninterpretable by anything except the task that trained it,
which defeats the point of `.nande` being an interchange format.

Also note this is what item 4 relies on: binding time is *derived* from position in the tree, so
any case where position and binding disagree needs to be explicit rather than assumed.

### General rule worth adding to Settled if mutant accepts item 1

**A component lives at the level where its inputs are scoped.** It predicts item 1's placement
(evidence has Individual scope, so an evidence-consuming mutator belongs at Individuals), it
predicts mutant's own reasoning on item 7 (a trace travelling up several levels is a sign of
misplacement, not a transport problem), and it is the general form of data rule 3. Had we been
using it, the backprop placement question would not have come up.
