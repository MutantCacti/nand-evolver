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
- **4a.** **A.** One flag on the dataset declaration controls both: `sequential` = arena persists across examples and no shuffle; `iid` = arena zeroed per example and shuffled. **B.** Two independent flags.
  **Rec: A.** Out-of-sync settings fail silently: a persistent arena plus shuffling makes fitness depend on the permutation.
- **4b.** Offspring arena: **A.** blank (Baldwinian). **B.** copy the parent's (Lamarckian).
  **Rec: A.** It's moot under iid. In lane layout "the parent's arena" is 64 arenas, so copying isn't well defined.

### 5. Task interface
- **A.** Two headers: `dataset.h` (load, indexed access `examples(seed, epoch, index)`, `sequential|iid` flag) and `task.h` (`score(outputs, expected, active) → {error, error_max}`). Datasets may be reused across tasks.
- **B.** One `task.h` per dataset covering both access and scoring.

**Rec: A.** It's mutant's own split (batching is preparation, the task is scoring). `error_max` normalises across tasks and short final batches.

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

<!-- THREAD appends review notes here -->
