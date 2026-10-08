# 2B Review of the stubs

mutant's review of `sem1-plan/nand-evolver/` (five items, wire session 5), and ATLAS's reading of each.
Drafted by ATLAS before DELTA's input, so the readings are one agent's and not yet joint.

**Status: 2B is dropped** (item 5). Nothing below should be implemented. The point of writing it
down is that item 5 reopens phase 1, and the other four items survive that reopening unchanged —
they are about the boundary, not the tree, so they should not be lost in the re-planning.

## 1. `parameter.seeds` → `experiment.seeds`

**Accepted, and it is a better category than the one it replaces.** A seed count is an instruction to
the Driver about how many Runs to perform. It is not a setting of the search: no C code ever reads
it, and `train` takes one seed per invocation.

The gain is in the hash definition. `parameter.seeds` forced the experiment hash to be stated as
"the parameter keys, except this one by name" — the only by-name carve-out in the whole scheme.
With an `experiment.` prefix the rule becomes a clean statement about prefixes:

> the experiment hash covers `protocol.`, `training.`, `inference.`, `parameter.` and `task.`,
> and excludes `experiment.` and `execution.` entirely.

So `experiment.` is "what the Driver does with this experiment", `execution.` is "how the machine
is used", and neither belongs in the identity of a result. Two prefixes out, no exceptions inside
the five that are in.

**One question:** is `experiment.seeds` a count (`5`, meaning seeds 0–4) or a list (`0, 7, 91`)? A
count is enough for P1 and keeps seeds derivable from nothing but the file. A list is needed only
if a particular seed must be reproduced after the fact, which the run directory already records.

## 2. `lane_*` / `packed_*` → `genome_*` / `model_*`

**Accepted at the interface.** The names were describing how each implementation stores a wire,
which is the one thing a caller cannot act on: you do not choose a layout, you choose by what you
are holding. `genome_harness_run` and `model_harness_run` say that, and they read consistently with
`core/genome.c`'s existing `genome_*` accessors rather than against them.

**Item 5 strengthens this.** `lane_harness_run` currently takes a non-`const Genome *` only because
the Trainer rewrites the genome mid-measurement. Once the Trainer moves out of the Harness (item 5
below), both functions take a read-only thing and the difference between them is *exactly* the type
— which is the condition mutant's item states.

**One exception to carry forward:** `tests/test_layouts.c` should keep its name. The public
interface should not mention layout, but the test exists *because* the two layouts could disagree —
that is its whole subject. Dropping the word there would also collapse the distinction from
`test_canonical.c`, which compares two Nand forms in one layout, not two layouts.

## 3. The reset record

mutant's objection is decisive against every in-band design: the input region must accept any bit
pattern, so no value in it can mean "reset", and length cannot mean it either without making a
short read indistinguishable from a control signal.

**The collision only exists in-band.** Framing removes it: a record is a small fixed header plus the
input region's bits, and the header carries the kind (`0` = input, `1` = reset). Nothing inside the
payload is reserved, so the arena still accepts every bit pattern.

This also fixes the timing worry rather than trading it away. A known record length is what makes a
reader robust: the length comes from the model file, so a partial read means "read more", never
"reset". Length-as-signal fails precisely because it destroys that property.

**Why framing rather than something out of band** (a signal, a second fd): the Dataset already
carries example boundaries explicitly, and a reset *is* an example boundary. Deployed input should
carry the same boundary information its training counterpart does, through the same channel, or the
two paths stop meaning the same thing.

**For P1 this stays unbuilt.** Ruling F is one process per example, so the kind field is always `0`
and a reset is what starting a process does. Framing is the design to adopt *if* streaming is built.

## 4. `Config` is a global, not a parameter

**Accepted; the catch is accurate.** `const Config *` is currently a parameter of six boundary
functions (`evolver_run`, `selector_select`, `mutator_mutate`, `exporter_export`, `feed_open`,
`config_load`) and two private ones. DECISIONS says config moves down read-only, written once, and
that the experiment file is embedded in the binary — and a `const` global, set once at start-up and
never written again, *is* that rule expressed in C. Threading a pointer through every call to say
"this is immutable and the same everywhere" is the weaker statement of it.

**The objection to check, and why it does not hold:** a global means one configuration per process,
which would matter if any test needed two. None can. Protocol and algorithm choices are `#define`s,
so a second configuration is a second binary — the differential test already compares two
implementations under *one* configuration, which is the only comparison that is meaningful.

So `config_load` fills the global, and the parameter comes off the other five.

## 5. There is no Individual level

**Accepted.** The level fails our own rule 3: every other level in the tree is named for a unit of
work (Run, Generation, Example, Round, Tick) and Individual is named for a bundle of data. Rule 2
says data never owns a loop. The genome is population state, owned at Generation; the arena is
scratch space, cleared per example. They were never one thing — they were a pair we drew once and
then kept.

**The doubt is already in the record.** REVIEW-1C's table of levels says the arena is "allocated
here (**or per worker**)" at Individual. That parenthesis is the whole of mutant's item: if the
owning level cannot say whether it owns the memory, it is not the owning level.

### Why the Trainer does not bring the level back

This is the obvious objection to dropping it, so it should be answered in writing. With a Trainer,
a genome's examples are a sequence rather than a set — the genome changes as it goes, so its
examples cannot be split across workers or reordered. That looks like a level whose loop is "the
examples of one genome, in order".

It is not a level. It is a **constraint on how the Generation's work may be split**:

- no Trainer: `(genome, example)` pairs are independent, and the splitter may divide them any way.
- with a Trainer: pairs sharing a genome stay together, in order.

Same loop, same owner, different splitting rule — and rule 2 already requires that the split be
decided in exactly one place. So the Trainer needs no level of its own, and the tree stops changing
shape between configurations, which is what `#` was introduced to avoid.

### What this makes the tree

```
Study (Driver)
└─* Experiment (Driver)
    └─* Run (Evolver, Exporter, Logger)
        └─* Generation (Evolver, Selector, Mutator, Trainer)   # Genome, Arena
            └─* Example (Harness)                              # Arena
                └─* Round (Kernel, Verifier)                   # Arena
                    └─* Tick (Kernel)                          # Arena
                        └─* Instruction                        # Arena
```

The Evolver's Generation loop runs the flattened `(genome, example)` product, which is the unit
mutant wants to thread. The arena belongs to the splitter, because it is per-worker scratch reused
across pairs — which is why `core/arena.h` has no `.c`. The Trainer joins the Selector and Mutator
as a component the Evolver *calls*: all three change a genome, differing only in their evidence.

**The two trees converge.** Train's Harness now owns Example and Round; deployed, it owns Example
and Round under a Deployment level whose only job is to create the arena once. The arena is created
by whoever owns the level above the Harness in both programs. That is a sharper version of "the
Harness is the same component in both" than the old tree could state.

### Blast radius, honestly

Dropping the level costs less in the stubs than "drop 2B" suggests, and that is worth saying so the
re-planning is not more expensive than it needs to be. The Harness never had a function at the
Individual level to delete: `harness_run(genome, arena, feed, tick_limit)` means "run these examples
on this genome", which is what it should still mean. What actually changes is `ARCHITECTURE.md`'s
tree, the Trainer's home, who allocates the arena, and the Evolver's split — the documents, and
`evolver.c`'s shape.

### A conflict this exposes

Taking the Trainer seriously surfaces something the old tree hid. In lane layout one pass evaluates
`lane_width` examples at once, so a Trainer cannot update "between examples" inside a lane group:
its finest real interval is one lane group. But `execution.lane_width` is an execution key, and
execution keys must never change results. Updating per lane group makes lane width change the
update schedule, and so the results — which `test_determinism` is built to catch.

The resolution I would propose: the Trainer's update interval is a `training.` key counted in
examples, and the build refuses a lane width that does not divide it. Then the interval is a stated
part of the experiment, and lane width stays free to vary within its divisors. **Not ruled** — it
is mutant's call, and it may be an argument for something else entirely.

## Open question for mutant

"Drop 2B" — does `sem1-plan/nand-evolver/` get deleted, or kept as the artifact of a superseded
plan? It is in git history either way, so nothing is lost by deleting it, but the answer changes
whether the re-planning starts from a blank directory or edits what is there. Not guessing.
