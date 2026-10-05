# Context Annotation (draft)

Intended to be appended to `ARCHITECTURE.md` once mutant confirms Q2 is closed. Drafted by THREAD
for DELTA's review. It records what the diagrams cannot show: how to read them, what generated
them, and which mistakes they are shaped to prevent.

## Reading the trees

- `*` a loop over many; `(` the executor that varies state; `#` the state varied.
- **The trees are accurate across variations, not for P1.** A level lists state that *any*
  variation varies there. `# Arena` at Generation is correct because offspring arena inheritance
  (the Lamarckian variant) varies it there, even though P1 does not. Do not "fix" a level because
  nothing touches that state in the reference build.
- Train and infer are separate trees because their loop structure genuinely differs — infer has no
  Generation level at all. Shared components are the *protocol* ones, not the loop owners.

## Who owns what

Four components own loops. Nothing exists only to loop.

| Component | Owns | Job |
|---|---|---|
| **Driver** | Study → Experiments → Runs | Tooling, in Python, outside the C program. Exempt from the loop rule. |
| **Evolver** | Generations → Individuals | Runs evolution; calls Selector then Mutator after the Individuals loop. |
| **Tester** / **Inferrer** | a given example range → Rounds | Drives inputs, observes, checks (Tester) or consumes (Inferrer). Calls the Trainer between examples, the Decoder per round. |
| **Kernel** | Ticks → Instructions | Ticks to ready or `max_ticks`, evaluating every instruction and writing back by seniority. |

Called at a point, owning no loop: **Encoder** (once, at Experiment), **Selector** and **Mutator**
(after the Individuals loop), **Decoder** (per round), **Trainer** (between examples; never in P1).

**The Tester is range-agnostic.** It loops the example range it is handed, never "all examples".
The work-splitter chooses the chunk — one example in P1, giving the fully flattened
(individual, example) product; all examples in individual mode, making Examples serial inside the
chunk while Individuals stays parallel. The Tester's code is identical in both. This is the single
site of the one parallel/serial declaration that varies.

## The rules that produced this structure

1. **Only agentive nouns own loops.** Stateful nouns (Source, Dataset, Arena, Genome, Rng) are
   functionality that loops call.
2. **A component may own adjacent loops.** The real constraint is narrower: a loop whose
   parallel/serial declaration *can vary* must have exactly one declaration site. Only
   Individual → Examples varies.
3. **Name the owner, not the state.** If the best name for a loop's owner is a noun for the state
   it tracks, the loop belongs to whoever owns that state. This retired a proposed `Clock` (the
   tick counter is the Kernel's own state, so the Kernel owns the tick loop), an `Evaluator`, and a
   `Sequencer`. A component whose entire content is a loop is not a responsibility.
4. **One level per function**, so the nest is the call graph and the loop tree predicts the files.
5. **Data rules.** Config moves *down*, read-only, written once at Experiment (the experiment
   file). Results move *up* one level at a time, **unreduced** until the Selector, which owns every
   reduction over examples because reduction is selection policy. Buffers are allocated at their
   owning level and written below it by pointer.

Rules 1–3 were each derived from a naming problem rather than from analysis. That is not
incidental: in this project **naming has been the diagnostic instrument**, and a name that feels
like filler has twice indicated a structural error. Treat an awkward component name as evidence,
not as a cosmetic matter.

## Scoping facts that are easy to get wrong

- **Source → Encoder → Dataset**, all at Experiment. `Source` is raw and task-specific; `Dataset`
  is encoded and **task-agnostic**. The Encoder is the only component that ever sees the world.
- **The Dataset is read-only everywhere below Experiment**, so it needs no per-worker copy. This is
  what makes the flattened parallel split safe.
- **Nothing prepares an order.** There is no permutation, no shuffle step, no cursor. The example
  is a pure lookup on `(seed, generation, example)[round]`. *Epoch* is not a concept here;
  generation subsumes it.
- **Arena spans three levels for one buffer:** allocated at Individual, reset at Example, written
  at Tick. One reset boundary only — the example. Every example is therefore self-contained,
  order-independent and parallelisable regardless of how many rounds it contains.
- **A round** is one input → ready → output handshake. An example contains 1..R rounds. `iid` vs
  `sequential` is just R = 1 vs R > 1, not a flag. Streaming is one long example.
- **Rng is Run-level**: a seed, with streams derived from `(seed, level indices)` at each draw site.
  There is no shared generator object, which is what makes reproducibility structural.
- **The Kernel emits raw per-lane ticks and reduces nothing.** One lane is one example, so those
  are already per-example values; the Tester attaches them to records unchanged and the **Selector**
  reduces, because which reduction counts is selection policy and lexicase selection needs them
  unreduced.

## The protocol boundary

- **Encoder and Decoder are protocol**: shared between train and infer, pinned in the model file,
  which carries the genome plus *both* ids and params. Many encoders may share a decoder.
- The **Decoder runs per round in both trees.** Its placement is independent of grading authority:
  the **Tester** owns the graded-rounds schedule; the Round never knows it. *(Open, not ruled:
  whether the Tester passes a read-only "is this round graded" flag down so the Decoder can skip
  ungraded rounds. Being told is not deciding, and it matters for value-attributable codecs, where
  decoding is a per-lane transpose and Sequential MNIST grades one round in twenty-eight. The
  ruled default is that the Decoder decodes every round, which also gives the Trainer per-round
  evidence.)*
- The Decoder owns **error attribution**: a per-bit significance at the output boundary, consumed
  by Trainer and Mutator as evidence. Each scheme declares its kind: *bit-attributable*
  (one-hot, thermometer — `out ^ expected` alone gives both which bits are wrong and which way to
  move, so it stays lane-parallel) or *value-attributable* (binary, Gray, float — fixes are not
  bit-local, so it decodes per lane).
- Only the **Decoder** has a true inverse pair (`encode_target` / `decode`), so the round-trip test
  targets the Decoder alone. The Encoder has no inverse; nothing decodes an input.
- `score()` measures **correctness only**. Cost terms — ticks, live gates, address space — travel up
  and are weighted at selection, because cost weights anneal and cannot be baked in below the level
  that anneals them. Cost prices *live* gates, never *present* ones, or it would delete the inert
  reservoir that gate seniority exists to protect.

## Deliberately absent

- **The parallel/serial partition**, because it is variation-dependent and the trees are meant to
  hold across variations. It lives in the declaration site instead.
- **The ready wire**, as protocol detail rather than data flow.
- **Batching**, as an execution concern. Lane width is an execution variation and encoding is
  protocol, so an Encoder that batched would need a different Encoder for the packed path.

## Still hypotheses, not settled

- A component lives at the level where its inputs are scoped.
- Evolver versus Selector + Mutator as the right factoring.
- Many encoders sharing one decoder.

These are to be settled by contact with implementation, not by argument.
