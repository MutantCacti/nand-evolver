# 1C Review of ARCHITECTURE.md

DELTA and THREAD's joint answers to mutant's five questions on `ARCHITECTURE.md` (`f843ea9`).
Drafted by DELTA, reviewed by THREAD.

**Scope note:** the 1C deliverable is the context annotation appended to `ARCHITECTURE.md`. Several answers below change what that annotation would say (Decoder level, Arena level, where data sits), so we hold the annotation until mutant rules on these.

## Q1. Which comes first, the encoder or the dataset?

- **The raw data comes first; the Encoder consumes it.** Both are **Experiment-scoped**: the encoded array depends only on (data, encoder params), which are identical across an experiment's replicates. Only the **order** varies per Run. So `Run (Dataset, Rng)` is misplaced: Run owns the Rng and therefore the order, not the data.
- **Labels are encoded at the same time**, by the Decoder's `encode_target`. A dataset becomes a list of examples, each a list of encoded rounds (input bits, optional expected bits).
- **Once encoded, everything below is task-agnostic.** The Encoder (and `encode_target`) are the only components that ever see the world.
- **The Encoder should not batch.** Packing 64 examples into words is layout, an execution variation (lane vs packed). Encoding is protocol (world → wire bits for one example). An encoder that batched would need a different encoder for the packed infer path. Batching is the lane implementation's job at the Individual → Example boundary.
- **Naming question:** what deserves the short name is the encoded array that flows down and is read in the hot loops. Proposal: **Source** (raw, task-specific) → Encoder → **Dataset** (encoded, task-agnostic).

## Q2. Which components loop, and which are called by loops?

**Every level in the tree is a loop**, and by "one level per function" each needs a named driver. The diagram names components but not the drivers, and the drivers are what become files and functions in 2A. The parenthesised names are of three kinds:

1. **data** owned at the level
2. **components called** at a point in the level's loop
3. one level whose loop body *is* the component (Kernel)

| Level | Loops over | Data (allocated / reset / written) | Called at this level |
|---|---|---|---|
| Experiment | stages, then Runs (replicates) | Source, encoded Dataset (once) | Encoder + `encode_target` (once, before the loop) |
| Run | Generations | Rng (seed) → order | — |
| Generation | Individuals | population records | **after** the loop: Selector once, Mutator per child |
| Individual | Examples | Genome; Arena **allocated** here (or per worker) | **between** examples: Trainer (individual mode) |
| Example | Rounds | Arena **reset** here; per-round output bits | **after** graded rounds: Decoder (attribution) + Verifier (error) |
| Round | Ticks | — | before: write inputs. Loop until ready or `max_ticks`. After: latch outputs |
| Tick | Instructions | Arena **written** here | writeback (seniority), ready check |
| Instruction | — | reads Genome + Arena | Kernel (the NAND) |

- **Two kinds of component.** *Variers* act per iteration and vary the level below (Selector, Mutator, Trainer, Kernel). *Establishers* run once and set up the level below (the Encoder, Dataset loading). mutant's labelling rule ("each component varies the loop underneath") holds for variers only. The Encoder varies nothing, which is why its placement looked odd.
- **The Kernel and the Instruction loop.** `tick()` owns the loop and `kernel()` is the body. Whether an implementation fuses them into one vectorised pass is an **execution** concern, invisible to the structure. That's "SIMD without being designed for it" in its cleanest test case.
- **Arena is three levels for one buffer** (data rule 3): allocated at Individual, reset at Example, written at Tick. Placing it at Tick misstates its lifetime. Placing it at Example, read literally, implies allocating once per example.
- **Genome:** owned at Individual, written by the Mutator (Generation) and the Trainer (Individual), read at Instruction.

## Q3. Decoder at the end of each round, or at the example?

**At the Example.** If the Decoder sits at the round boundary, the Round must know the grading schedule, which is the leak you flagged.

- **Round** owns only the handshake: write inputs, tick until ready or `max_ticks`, latch output bits, return them upward. It is ignorant of grading.
- **Example** owns the schedule (Settled: the example declares its graded rounds). After its rounds it calls the Decoder (attribution) and Verifier (error) on graded rounds only. The cost is R × output words held per example (Sequential MNIST: 28).
- **Infer differs only in the caller:** the world consumes every round's output, so infer decodes every round. Same component.
- **Decoder vs Verifier stay separate.** The Decoder is protocol: pinned in the model file, linked into `infer`. The Verifier is task: train-only, scores against labels.

## Q4. What do you call the component(s) that dispatch experiments?

- **Driver:** executes one experiment file's stages (build train → Runs → build infer → infer → report). Already the term in DECISIONS.
- **Study:** a set of experiment files, i.e. one target figure. A Study loops over Experiments. We avoid *workbench*, which is mutant's word for the codebase.

## Q5. Problems in the diagram, or the structure it reveals

1. **Experiment is a pipeline, not just a loop over Runs.** Stages: build train → Runs → build infer → infer → report. The tree shows only train. Either label it the train tree or add the infer branch. Without that, the Encoder/Decoder read as training-only concerns, which is backwards: they are protocol and are linked into `infer`.
2. **Nouns and verbs are mixed in the parentheses** (Q2), and Arena is at the wrong level.
3. **Selector is fed directly by Verifier, skipping the Individual reduction.** Results move up one level, so per-example results reduce at Individual into the record the Selector reads.
   - The record also carries the cost terms, which don't come from the Verifier: ticks come from Round/Tick, live gates from the Genome.
   - None of those arrows exist.
4. **Dataset's arrow lands in the Tick box.** Inputs are written once per **Round** (handshake). The expected bits must reach the Verifier, and there's no arrow for that.
5. **Rng only feeds Dataset.** The Mutator, Selector (tournament) and Trainer all draw randomness, from streams derived from (seed, level indices) at the draw site.
6. **Trainer evidence comes only from Verifier.** Settled also gives it the Decoder's per-bit attribution (and, later, per-Nand stats from the Kernel).
7. **Ready isn't shown.** The Round's loop condition reads the ready wire in the Arena (protocol).
8. **The declared parallel/serial partition isn't shown.** Annotating each level per mode (P1: Ticks and Generations serial; individual mode: Examples serial too) would make the partition visible where the work-splitter needs it.
9. "Examples (Verifier)" should be singular, like the other levels.

## THREAD

Reviewed; agreed throughout. Four additions, the first being a synthesis rather than a new point.

### The corrected loop tree

Q5 lists nine faults but doesn't assemble the result. Applying all of them, and folding in Q4's
naming:

```
Study                                       * Experiments — one target figure
└─* Experiment  (Driver; Encoder)             stages, then Runs. Source → Dataset, once
    └─* Run     (Rng → order)                 order established here, once
        └─* Generation (Selector, Mutator)    both called after the loop
            └─* Individual (Genome, Trainer)  Arena allocated
                └─* Example (Verifier, Decoder)  Arena reset; graded rounds only
                    └─* Round (Handshake)        inputs in, tick to ready, latch out
                        └─* Tick (Arena)          writeback by seniority, ready check
                            └─* Instruction (Kernel)
```

Changes from `ARCHITECTURE.md`: **Study** added above Experiment (Q4); **Dataset** moved from Run
to Experiment (Q1); **Decoder** moved from Round to Example, and **Handshake** restored as the
Round component (Q3); Arena's three levels annotated (Q2); *Examples* singular. The Experiment
line is still an approximation, since it sequences stages as well as looping Runs (Q5.1).

### This supersedes a Settled line in DECISIONS.md

Q3's answer contradicts the round-3 Settled entry, which places attribution **"at the output
boundary, once per round"** and has value-attributable codecs decoding "once per round, never per
tick". If the Decoder moves to the Example, that becomes *once per graded round, called from the
Example*. Worth editing when mutant rules, or the two documents will disagree on the one point
most likely to be read in isolation by a future agent.

### Why putting the Dataset at Experiment pays off twice

Beyond correct scoping: the encoded Dataset is **read-only everywhere below Experiment**, so it
is config in the sense of data rule 1 and needs **no per-worker copy** — unlike the Arena, which
is per-individual mutable state. That is precisely what makes the flattened (genome, example)
parallel split safe, and it would not hold if the Dataset were re-derived per Run.

### Variers vs establishers is general, not an Encoder quirk

The table shows nothing called at Run, but the **order** has to be computed there — so Run has an
establisher too, exactly as Experiment has the Encoder. Two levels, same pattern: a once-per-
iteration setup step that varies nothing below it. That's worth stating, since presented only via
the Encoder it reads as a special case rather than a second kind of component.

## Round 2 (mutant's notes, 2026-10-05; DELTA response)

**Rulings recorded:**
- Q1: Source > Encoder > Dataset.
- Q3: Decoder after the Example.
- Q4: Driver.
- Q5: train and infer trees separated; executor `(` and state `#` marked separately; Rng readable by everything below it (arrow into a box = read-only access); Decoder → Trainer arrow added; ready and the declared partition left off by design (protocol detail; variation-dependent).
- Naming constraint: **only an agentive noun (component/file) owns a loop; stateful nouns are called by loops.**

### Note 1: order is a lookup, not a stage
Agreed, and it retracts THREAD's "Run has an establisher too". The example at any point is a pure lookup, `Dataset[index(seed, generation, example)][round]`, so nothing builds an ordered structure. Keying on `generation` means every individual in a generation sees the same examples (a fair comparison) and the examples change between generations. Pure lookup is also what makes the flattened parallel split safe.

### Note 2: Tester instead of Verifier
**Yes.** A component that drives inputs, observes outputs and checks them against expected values is a *testbench* in hardware terms, so **Tester** is the accurate name. One consequence: infer drives inputs too, so the Inferrer is the infer-side counterpart (drive + decode, no check). Driving inputs is protocol; checking is task. That split falls on the Tester/Inferrer boundary rather than inside either.

### Q5.3: who reduces per-example results? Rec: the Selector
The reduction (mean error, worst case, error-then-cost, ...) is selection *policy*, so it carries algorithm and parameter variation.
- If the Tester reduces, selection policy leaks below Generation.
- **Lexicase selection** (Spector), a standard GP method, selects on *unreduced* per-example errors. A Tester-side reduction would make it impossible to express.
- So the Tester emits raw per-example records, and the Selector reduces (or doesn't).
- Cost: individuals × batches records per generation, which is small at 64 examples per lane batch.

### Q2 re-asked: which agentive component owns each loop?
Applying the constraint to the current train tree leaves loops whose owner is a stateful noun or nothing:

| Loop (level iterates) | Current owner | Problem | Proposal |
|---|---|---|---|
| Experiments (Study) | Driver | — | Driver |
| Runs (Experiment) | Encoder | Encoder runs once and doesn't iterate Runs | Driver also iterates replicates? Or a separate agent |
| Generations (Run) | Rng | stateful noun, now just a lookup (note 1) | **Evolver**: iterates generations, calls Selector + Mutator. Your original top-level name, and it keeps the tracked Evolver vs Selector+Mutator question in one place |
| Individuals (Generation) | Selector, Mutator | they run *after* the loop, not as it | the work splitter dispatching individuals in parallel; name open (Dispatcher?) |
| Examples (Individual) | Trainer | population mode has no Trainer | Tester iterates examples; the Trainer is called between them in individual mode |
| Rounds (Example) | Verifier/Decoder | — | Tester |
| Ticks (Round) | none | — | Tester (drive, tick to ready, latch) |
| Instructions (Tick) | Kernel | — | Kernel |

Open: is one agent owning several adjacent loops (Tester: Examples, Rounds, Ticks; Driver: Experiments, Runs) acceptable? Or must each loop have its own agent, i.e. one level per file?

### Inconsistencies remaining in ARCHITECTURE.md (dd1b778)
1. **Data flow diagram: Decoder is still inside the Round box,** contradicting the Q3 ruling (after the Example).
2. **`Run (Rng)` puts a stateful noun in executor position,** against the naming constraint (see the table).
3. **`# Arena` at Generation:** Generation's executors (Selector, Mutator) vary Genome, not Arena.
4. **Infer tree has no Decoder,** but infer decodes every round. It belongs at Round under the Inferrer (or wherever the Inferrer's loop sits).
5. **DECISIONS.md round-3 Settled** still says attribution happens "at the output boundary, once per round". It should become "after the Example, on graded rounds".
