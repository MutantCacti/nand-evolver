# SYN 1B: Decisions

Priority-ordered A/B decisions for `mutant`. Each item: question, options, recommendation, why.
`^&` marks a constraint from Prof. Jefferson. Drafted by DELTA, reviewed by THREAD.

## Settled in 1A (veto if wrong)

- **Hierarchy** (mutant's): Run → Generations (Selector, Mutator) → Individuals (Genome) → Examples (Trial) → Ticks (Arena) → Instructions (Kernel). Each component varies the loop below it.
- **Data rules:**
  1. Config moves down, read-only, written once at Run ^&.
  2. Results move up one level, reduced at each boundary.
  3. Buffers are allocated at their owning level and written below it by pointer (Arena, and any added noun).
- **One level per function.** `main` is the whole pipeline in a few tiered loops.
- **Selector** is `scores → indices` and never reads a genome. **Mutator** is `genome → genome` and never reads a score.
- **Ready is protocol, not task.** The kernel reads its own halt from the Arena. Ready initialises to all-ones (active low).
- **Advance mask** replaces "lane retirement". It marks the lanes due their next row (handshake).
- **Batching is dataset preparation; Task is scoring, one file per dataset behind a shared header.** Task is not a hierarchy level.
- **Example source is indexed, not walked:** `(seed, epoch, index) → examples`.
- **RNG:** no shared generator object. Streams are derived from `(seed, level indices)` at the draw site, so the Mutator is pure `(genome, seed, gen, individual) → genome'`.
- **Vocabulary:** "frontier" is retired. Use *active set* (gates executing this tick), *clocked/multi-rate* (tick modulo), *event-driven* (next-index), *quiescent* (empty active set), and *scoring* for the meaning boundary. No name is reused across levels.
- **P1:** every gate every tick. Population mode is tournament + elites (elite = identity mutation). Inputs are reserved.

## Decisions

### 1. How do variants attach and compose? (Goal 2)
Composable at will ^&. A variant that changes a struct layout or a hot loop cannot be a runtime branch without costing the kernel.
- **A.** Structural variants are **compile-time pairs** (`-D` flags into a single read-only config header): gene schema + kernel (tick modulo, next-index), trace + backprop mutator. Scalars, rates and registry names (task, selector, mutator) are runtime JSON read at Run.
- **B.** Everything is runtime (function-pointer registries, union/optional fields in Genome), with one binary.
- **C.** One binary per variant combination, selected by Makefile targets only, with no in-source `#if`.

**Rec: A.** It keeps the kernel branch-free and gives Make one axis per structural variant, so combinations are build matrix entries for P2 comparisons. Needs a follow-up: which variants are mutually exclusive (scheduling: every-tick | clocked | event-driven) and which are paired.

### 2. Is the parallel/serial partition a per-mode declaration?
The flattened (genome, batch) split ^& assumes Examples are parallel. Individual mode mutates the genome between batches, making Examples serial within an individual.
- **A.** Each mode declares which levels are parallel. The work-splitter flattens whatever is declared parallel. P1: Instructions/Examples/Individuals parallel, Ticks/Generations serial. Individual mode: Examples serial.
- **B.** The partition is fixed. Individual mode is restructured to fit it (e.g. it mutates only between generations).

**Rec: A.** Parallelism is a loop transformation over declared loops ("SIMD without being designed for it"). Individual mode at population 1 still parallelises over K independent replicas.

### 3. Skipped gate (tick modulo): does it hold its wire?
- **A.** Hold. A senior's wire keeps its value on skipped ticks and juniors can never write it. In train, colliding juniors are **kept and ignored** (the neutral-drift reservoir). At export they are **pruned** (canonicalisation stays a pass).
- **B.** Release. Any gate may write a wire on ticks its senior skips (time-multiplexed wires).

**Rec: A.** It preserves both documented invariants (adding a gate is neutral, README canonicalisation). B is more expressive but makes adds conditionally destructive and needs a clock-aware canonicaliser.

### 4. Arena persistence, shuffle, offspring arena
- **4a.** There are **three** arena boundaries, not one: **rows within an example** (must persist — that is what the handshake *is*), **examples within a lifetime** (the real question here), and **offspring** (4b).
  **A.** One flag on the dataset declaration: `iid` = single-row examples, arena zeroed per example, shuffled; `sequential` = multi-row examples, arena persists across rows, zeroed between examples, not shuffled. **B.** Two independent flags (persistence, shuffle).
  **Rec: A**, with the row boundary outside the flag because it is definitional rather than chosen. Out-of-sync settings fail silently: a persistent arena plus shuffling makes fitness depend on the permutation. P3 streaming would later add a third value that also persists *across* examples; it is not a fourth boundary.
- **4b.** Offspring arena: **A.** blank (Baldwinian). **B.** copy the parent's (Lamarckian).
  **Rec: A.** It's moot under iid. In lane layout "the parent's arena" is 64 arenas, so copying isn't well defined.

### 5. Task interface
- **A.** Two headers: `dataset.h` (load, indexed access `examples(seed, epoch, index)`, `sequential|iid` flag) and `task.h` (`score(outputs, expected, active) → {error, error_max}`). Datasets may be reused across tasks.
- **B.** One `task.h` per dataset covering both access and scoring.

**Rec: A.** It's mutant's own split (batching is preparation, the task is scoring). `error_max` normalises across tasks and short final batches.

**An example is not always one row.** Under `sequential` one example is a *row sequence with a
single label* (Sequential MNIST: 28 rows, one digit), so the accessor must yield a length, not
just a row, and **scoring happens at the end of a sequence, not per row**. Note the Settled
advance mask already presumes multi-row examples — lanes due their next row — so the accessor
has to supply them. For `iid` the length is 1 and nothing changes.

### 6. Grading defaults and the tick budget
- **6a.** The default output encoding is keyed to the output's measurement scale. The task author picks; these are the library defaults:
  - **nominal** (classes, incl. MNIST digits): one-hot / group-sum + argmax
  - **ordinal or small range**: thermometer (Hamming = |v−w| exactly)
  - **large range**: Gray (locally smooth, caveat: extremes look close)

  **A.** Ship these three as scorer helpers. **B.** Raw per-bit Hamming only; tasks build their own.
  **Rec: A.**
- **6b.** `max_ticks` is a **depth ceiling**, not a safety bound. Minimum ticks to ready ≥ circuit depth, so a tight budget or a tick penalty hides deep solutions (XOR optimum needs ≥3). **A.** First-class hyperparameter, logged with every run. **B.** Fixed per task.
  **Rec: A.**

### 7. Search-backprop trace: retain or recompute?
The kernel is deterministic and the example source is pure, so the trace is reproducible.
- **A.** Recompute: re-run only the individuals the Mutator wants traces for. The default build stays trace-free.
- **B.** Retain `T×W` words per work item during evaluation.

**Rec: A, with a caveat.** Under `sequential` datasets or individual mode, recomputing example k replays the whole lifetime up to k. Revisit when P2(c) is built.

### 8. Address-space slack (`i+1+2N`)
Slack sets the **inert-add rate**: a random output address lands on an occupied wire with probability ≈ occupied/space, and a new gate always loses that collision.
- **A.** Runtime scalar (default factor 2), documented as the inert-add rate.
- **B.** Fixed at 2N as now.

**Rec: A.**

### 9. Selector input: absolute score only, or with history?
Needed for delta-error / lifetime-error experiments (P2).
- **A.** The score record carries optional history (parent score, lifetime mean). The Selector stays pure `records → indices`.
- **B.** Absolute score only. History-based selection is a separate Selector with its own state.

**Rec: A.** History is data with Generations scope, consistent with data rule 3.

## THREAD

Reviewed, agreed with everything above. MNIST-is-nominal correction accepted — my slip, my own
taxonomy contradicted itself. Two direct edits, three additions to Settled, one new item, one note.

### Direct edits made

- **4a, rewritten.** It named one boundary where there are three. Rows-within-an-example must
  persist (that is the handshake), examples-within-a-lifetime is the actual question, offspring
  is 4b. Options unchanged; the recommendation now excludes the row boundary as definitional.
- **5, extended.** The accessor had no notion of a multi-row example, but the Settled advance
  mask already presumes one. Added: under `sequential` an example is a row sequence with a
  single label, so the accessor must yield a length and scoring happens at end-of-sequence.

### Additions to "Settled in 1A"

- **`score()` measures correctness only.** Cost terms — ticks, live gates, address space — travel
  up in the Trial/Measurement record and are weighted at the **Selector**, never folded into
  `error`. This follows from "never collapse to a scalar below selection": cost weights anneal,
  so they cannot be baked in below the level that anneals them.
- **Cost terms price *live* gates, not *present* gates.** Interaction between items 3 and 8:
  hold-and-ignore means a large fraction of gates are inert **by design**, so a size penalty on
  present gates would delete the neutral-drift reservoir that 3A exists to protect. Measurement
  needs both counts under distinct names, and only the live one may carry a cost weight.
- **The declared partition (item 2) is testable, and the test is determinism under thread
  count:** same seed, 1 thread vs N threads, bit-identical results. One assertion covers three
  invariants at once — declared-parallel levels have no hidden dependency, no shared RNG, and the
  example source is indexed rather than walked. Hand it to DESTUB at 2C.

### 10. The CLI half of Goal 2: what happens when the two axes disagree?

Item 1 settles the build axis (`-D` structure) and the runtime axis (JSON scalars). Neither
covers their **interaction**, which is where P2 comparison work will actually break: a config
naming a variant the binary wasn't compiled with.

- **A.** The binary reports its compiled-in variant set (`nande-train --caps`), a config naming
  an absent variant **fails loudly at Run**, and the build identity (the `-D` set) is embedded
  in the binary and emitted with every run alongside the runtime config.
- **B.** Silent fallback to the compiled default, with a warning.
- **C.** No introspection; the build matrix is tracked outside the program, in the Makefile and
  run logs.

**Rec: A.** Jefferson's "compare stepped improvements" ^& requires every result to be
attributable to a `(build identity, runtime config)` pair. A silent fallback makes a build-matrix
comparison quietly meaningless rather than loudly broken, which is the worst available failure
mode for the one thing the project is graded on.

### Note, not a decision: the stop condition

Run owns it, and it is the only piece of Run-level state nobody has specified. Recommend a
generations cap plus optional early stop on zero error, **computed at Generations and reduced to
the single bit that reaches Run** — otherwise Run needs the score vector and starts doing
selection, which is the exact mislabelling mutant caught when deriving the hierarchy.
