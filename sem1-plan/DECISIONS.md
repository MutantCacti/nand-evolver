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
- ~~Training never decodes.~~ **Retracted** (mutant, 17:30): a Trainer can't backpropagate without knowing which output bits matter. For a float, a sign or exponent bit outweighs a mantissa LSB. Revised:
  - **The Codec owns error attribution as well as encoding.** Besides `encode_target`, it gives each output bit its **significance**: how much the decoded error changes if that bit flips (a discrete gradient at the output boundary). That is the first step of any backward search, and the Trainer and Mutator consume it as evidence.
  - **Separable codecs** (one-hot, thermometer; Gray only approximately): significance is a constant per bit, so scoring and attribution stay in bit space and lane-parallel.
  - **Non-separable codecs** (binary integers, floats): significance depends on the value (a float mantissa bit's weight depends on its exponent). They must decode per lane, transposing out of lane form at roughly a lane-width cost. That cost is paid at the output boundary only, once per round, not per tick.
  - The codec **declares** which kind it is, so the cost is visible in the experiment file rather than discovered.
- **Same code, different call levels.** In train, **encoding** runs once, at Run: the whole dataset and its labels are encoded to bit form at load (the inputs are dataset-scoped). **Attribution** runs at the output boundary of each round (the inputs are round-scoped). In run/, encode and decode run per round. The kernel's hot loops never touch the codec.
- **Installation:** run binary = codec (front and back) + packed kernel + embedded genome blob.

Options:
- **A.** One Codec component, two directions. Encode and decode in one file make inverse consistency a local, testable property (`decode(encode_target(v)) == v`).
- **B.** Separate Encoder and Decoder components (independently swappable, e.g. a thermometer input with one-hot output).

**Rec: A**, with input and output codecs selected independently inside it, so B's flexibility comes back as two parameters rather than two components.

### 2. Scripting interface (the driver)

mutant: *"If a top-level descriptor file uniquely identifies a build and its runtime parameters,
then both the build and run must take in the same descriptor, so the discipline issue is not
resolved."*

**Conceded — round 2's item 4 claim was too strong.** Config-above-build removes the
compile-vs-runtime mismatch and replaces it with a descriptor-vs-descriptor mismatch: two
invocations each naming a file, free to name different files. The hash check catches that, but
catching is discipline, not construction.

The resolution is mutant's own next sentence. If a pipeline is *compile (train) → execute (train)
→ compile (run) → report*, those are **stages of one invocation**, not separate invocations. So:

> **Invariant: the descriptor crosses the human boundary exactly once.** Every later consumer
> receives it from the driver, never from a person.

Which makes the binaries' interface a consequence rather than a choice: **the C binaries must not
accept a descriptor path at all**, only the trivial payload the driver derives (DELTA's key=value
form — C never parses the descriptor format). There is then no second place to type a filename.
The embedded hash check stays, but as defence in depth for hand-invocation during development,
not as the mechanism. **Test of any proposed interface: count the places a human can name a
descriptor. More than one and discipline is back.**

**Stages, keyed by their inputs** — this is the mechanism that makes ruling 4B pay off:

| # | Stage | Keyed by | Required? |
|---|---|---|---|
| 1 | compile (train) | hash(protocol + algorithm subtree, commit) | yes |
| 2 | execute (train) → model file + series | hash(1, parameter subtree, seed) | yes |
| 3 | compile (infer), genome installed into the codec | hash(2, codec id + params) | only with a deployment target |
| 4 | execute (infer) + perf/energy measurement | hash(3, held-out set) | only with a deployment target |
| 5 | report | hash(2, 4) | yes |

Content-addressing each stage means a hyperparameter sweep re-runs **stage 2 only**, reusing one
stage-1 binary across every point. **Stages 3–4 are conditional:** a training comparison needs
1, 2 and 5, so a descriptor with no deployment target should stop after stage 2 rather than build
an inference binary nobody loads. The driver should not be written assuming a fixed stage count.

**The report must identify what computed it, not only what was computed.** The descriptor fixes
the behavioural tree, but the **world axes are not in it** — and item 2 ruled that execution
variations change time and never results. The target figure's x-axis *is* time. So thread count,
machine, lane width, compiler and flags must be logged explicitly alongside the descriptor and
commit; otherwise two lines drawn from runs on different thread counts are silently
incomparable, in exactly the way the execution category exists to prevent. Descriptor = what was
computed. Report = what computed it. A point on the graph needs both.

Layout `runs/<descriptor-hash>/<seed>/`, with the descriptor and commit copied in.

Driver language:
- **A.** Python, thin. It already has to hash, drive Make, and plot — and plotting is Python
  regardless. mlql precedent.
- **B.** Make only.
- **C.** Shell.

**Rec: A.** The decisive point is that **the driver is never shipped**: it is a development tool,
so its language has no bearing on the MCU deployment target, which is the only place the
toolchain is constrained. B cannot reasonably express replicate loops, hashing and reporting; C
can, but badly.

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

Round 3 review. Item 2 written above. Agreed with items 1 and 3; DELTA's terminology catch is
better than mine (I'd only found two collisions on "run", not three). Three additions, one small
amendment, no edits to DELTA's items.

### Item 1 (Codec): agreed, Rec A, with two consequences

- **The Codec must live in `core/`.** It is shared by train and infer *by definition* — that is
  what makes encoding a protocol variation — so it cannot sit in either. That gives a concrete
  Phase 2A consequence: the inference binary is **kernel + codec + genome blob**, three parts, of
  which only the codec is task-specific. The kernel stays task-agnostic, which is the property
  worth protecting.
- **Sharpening "training never decodes":** correct for the hot path, but reports decode. So
  `decode` is absent from Examples-and-below, not from the train binary — which means the codec is
  linked into *both* binaries anyway, reinforcing the `core/` placement.
- **Two tests for DESTUB at 2C**, extending DELTA's round-trip: (i) `decode(encode_target(v)) == v`
  across the task domain, and (ii) **the same codec object linked into both binaries**, which
  stretches the existing lane-vs-packed differential test from the kernel to the whole I/O path.
  A deployed genome being uninterpretable is precisely a codec mismatch, so it deserves a test
  rather than a convention.

### Item 3 (Terminology): agreed, Rec A, one amendment

Accept *experiment* for the concept, **Run** staying as the hierarchy level, and `infer/` for the
inference binary. Small amendment: call the file a **manifest** rather than an *experiment file*.
One word, no collision, and it will appear constantly in prose and in code
(`load_manifest` reads better than `load_experiment_file`). "Experiment" then names the thing and
"manifest" names its definition, rather than one word doing both jobs.

Minor knock-on worth noting: renaming `run/` → `infer/` is also a documentation change —
`core/word.h`'s header comment already describes the two word semantics in terms of `run/` doing
bit-packed inference.

### For mutant

Two things in item 2 I'd most like checked, since both change what gets built rather than what
it's called:

1. **The binaries must not accept a descriptor path at all.** That's the structural form of your
   discipline objection, and it means the C side's config interface is a driver-emitted payload,
   never the manifest. Cheap now, awkward to retrofit.
2. **The report has to carry the world axes** (thread count, machine, lane width, compiler
   flags). They are deliberately outside the manifest, since they aren't model behaviour — but
   the figure's x-axis is time, so without them two lines aren't comparable.
