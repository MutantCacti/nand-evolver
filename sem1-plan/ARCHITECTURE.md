# Architecture

A summary of the planned nand-evolver data and responsibility structure.

## What the program does

nand-evolver searches for genomes (graphs of Nands) that solve a task. A **task** is one problem to be solved: its raw data, how that data is written onto wires, how wire values are read back as answers, and which answers count as correct. The search works by evolution: it keeps a collection of genomes, measures how well each one solves the task, keeps the better ones and makes randomly changed copies of them, and repeats. A genome found this way can then be run on its own, outside the search.

So there are two programs:
- **train** performs the search.
- **infer** is a finished genome running on input from the world.

## Responsibility Trees

Identify the series of nested loops that encapsulate stages of program execution.

`*` = Loops over many
`(` = Executor that varies state
`#` = State varied

Each line of a tree is a **level**, and **each level is one loop**. A level's `( )` lists every component that acts there: the level's **owner**, whose loop repeats the level beneath, and the components it calls. An owner therefore appears at every level whose loop it runs. A thing that is not looped over is not a level, however real it is.

A level's `#` lists everything that *any* configuration of the program might create or change there, not only what the simplest configuration does. For example, `# Arena` appears at Generation because one configuration lets a new genome start from a copy of its parent's memory space, even though the simplest configuration starts it empty.

### Training

```
Study (Driver)                                              # Source
└─* Experiment (Driver)                                     # Config, Dataset
    └─* Run (Evolver, Exporter, Logger)                     # Rng, Model, Log
        └─* Generation (Evolver, Selector, Mutator, Verifier, Trainer)
                                                            # Genome, Arena
            └─* Example (Harness)                           # Arena
                └─* Round (Kernel, Harness)                 # Arena
                    └─* Tick (Kernel)                       # Arena
                        └─* Instruction                     # Arena
```

### Deployment

```
Deployment (main)                           # Arena, Model
└─* Round (Kernel, Harness)                 # Arena
    └─* Tick (Kernel)                       # Arena
        └─* Instruction                     # Arena
```

The deployed program has no Study, Experiment, Run or Generation level, because an embedder shipping a model does not loop over experiments, seeds or candidate genomes. Those levels are the workbench's, not the product's.

**It has no Example level either, and that is not an omission.** An example is a lifetime (defined at the Example level below), and a lifetime is delimited rather than iterated. In train the Harness runs one example at a time, looping its rounds, so Example is a level there. Deployed, `main` loops over *records* and an example is the span between two resets, so there is nothing to loop over and no function to own — the Harness is entered a level lower, one round per record. The concept is unchanged in both programs; only in one of them is it a loop.

## Who owns the loops

Four components own all the loops. Every other component is *called* by one of them at a fixed point.

| Owner | Sits at | Repeats | Calls |
|---|---|---|---|
| **Driver** | Study, Experiment | Experiments, Runs | builds the Dataset once per experiment |
| **Evolver** | Run, Generation | Generations, work units | the Harness once per example; the Verifier on each graded round of what comes back; the Trainer between examples, where there is one; Selector then Mutator once a generation's work units are measured; the Exporter once, at the end of the run |
| **Harness** | Example | Rounds | the Kernel, once per round |
| **main** (infer) | Deployment | Records, and so Rounds | the Harness on each input record, and to reset on each reset record |
| **Kernel** | Round, Tick | Ticks, Instructions | none; it evaluates Nands directly |

**The Harness owns one level, Example, and it is the only code that knows the wire layout.** It is entered three ways, and each exists for a reason the others don't cover:

- **one example:** reset the memory space, then run every one of the example's rounds in order, keeping the space across them. This is the Example level, and it is train's entry, because there every round of an example is known in advance.
- **one round:** write the input values and ready's start value into the memory space, call the Kernel, read the output region back. This is the deployed program's entry, because there rounds arrive one at a time and an embedder reacting to an output needs control back between them. It is also the inner step of an example.
- **a reset:** put the memory space into its start-of-example state, which is passed in (nothing, or an inherited memory state). This is what a reset record asks for, and the first step of an example.

What it does *not* own is a loop over examples. That was the Individual level, and hiding it here was what stopped the Evolver dividing its work: a loop over rounds is inside one example and so inside one work unit, while a loop over examples is the work unit itself.

**It never learns that grading exists.** The Verifier and the Trainer are leaves the Evolver calls, on what the Harness hands back. So nothing on the path a genome walks knows which rounds are scored, which is part of why that path is identical in both programs.

**The Harness is the same component in both programs**, which is what makes a trained genome mean the same thing when deployed: every part of the path a genome experiences is shared code. It differs only in what it is handed, a Genome in train and a Model in infer.

At Round the owner is not the entry point, and this is the one place in the trees where that happens. The caller — the Harness in train, running an example's rounds; `main` in infer, running a record's — calls the Harness's round entry, which writes the inputs and ready's start value, calls the **Kernel**, whose loop over ticks is the Round level, and then reads the outputs.

## The levels

Top to bottom, for training.

**Study**. A set of experiments meant to be compared with each other, for example the lines of one plot. The **Driver** is a Python tool, separate from the C programs. For each experiment it builds the programs, runs them, collects their output and writes a report.
- `# Source`: the **Source** is the task's raw data, e.g. images and their **labels** (the correct answers).

**Experiment**. One fully specified search, described by one **experiment file**. That file is the **Config**: it fixes the task, every algorithm choice and every numeric setting. It is a workbench artifact; the deployed program never sees one.
- Choices that change the program's structure are compiled in, so each combination of them is its own binary. Numeric settings are read at start-up into one read-only global, never passed from function to function: configuration that cannot be changed and is the same everywhere is not an argument.
- The Driver converts the Source into the **Dataset**: a list of examples (defined below) expressed as wire values. Raw input data is flattened into bits and chunked into rounds; it is never encoded into an invented representation, so a genome must learn whatever encoding of its raw input suits it.
- The one output-side convention is the **target**: how a label becomes the expected values of the output wires, e.g. one wire per possible digit with the right digit's wire set, or a label's raw bits. The task names its target, and the Driver uses the same convention in reverse to read answers back.
- The Dataset is never changed after this, so every level below can read it freely without copying it.

**Run**. One complete search from one seed. The Driver repeats it per experiment to measure how much results vary between seeds. Only train has runs. The **Evolver** carries out a run, generation after generation. When the run ends, it hands the best genome it found to the **Exporter**, which canonicalises it (the README's optimised form, in a configured way) and writes it as a **model file**: the canonical genome, its input and output sizes, and optionally the memory state it should start from. That file is the deployed program's only input. Data leaves a run in one direction: Evolver → Exporter → model file.
- `# Rng`: the random seed. Every random choice anywhere below is computed from the seed plus its position, e.g. which example is used as the 40th example of generation 12. There is no stored random state that code shares or advances, so a run gives the same result however its work is divided between threads.
- For the same reason, examples are never shuffled into a stored order. The example to use is computed from (seed, generation, position within the generation).
- **Checkpoints** are the Evolver's own: the population, in training form, at a generation boundary, read back only by the Evolver to resume a run. They are never exported.
- The **Logger** keeps the **run log**, the record the Driver reads for reports and plots. Every component writes its own events to it. It is called, never a loop owner, and logging never changes results: a run's model file, checkpoints and per-generation records are identical however its work is divided between threads.
- **The log is ordered by wall time, and never out of generation order.** Only generations have a real order; an example's index within one is arbitrary, and may be produced in any order or in parallel, so ordering by it would claim a meaning it does not have. Per-thread buffers are merged at generation boundaries, which is where the whole population is at rest, so a generation's lines can never appear among the next generation's. The log itself is therefore not one of the things compared for identity between runs: within a generation, line order follows whichever thread got there first.

**Generation**. One step of evolution. The **population** (the current collection of genomes) is measured; then:
- the **Selector** compares the results and chooses which genomes become parents;
- the **Mutator** makes the next population by copying parents' genomes with random changes, e.g. adding a Nand or rewiring an index.

The **Verifier** and the **Trainer** are also called from here, per example rather than per generation: the Evolver hands an example to the Harness, and turns what comes back into an error. They are leaves — neither calls anything else — which is what lets the Harness stay ignorant of grading.

Measuring a generation is the one place work is divided. The Evolver flattens the generation into (genome, example) pairs and splits them; the piece of work it hands out is **one genome and a range of examples**, which is a **work unit**. A range of one is the reference. Nothing in a work unit reads another's state, so they can be measured in any order and in parallel.

- A genome and the memory space it is measured with are *not* one thing. The genome is population state, owned here; the memory space is a worker's scratch, reused from one work unit to the next and cleared at every example boundary. There is no level between Generation and Example.
- The **Trainer** is optional, and the configuration that has one is a variant of the Evolver. It changes a genome *during* measurement, between examples, using evidence from the results so far. That makes a genome's examples a sequence rather than a set, so the variant hands out whole sequences instead of splitting them freely — a constraint on how the work is divided, not a level of its own.

**Example**. One problem from the Dataset: a sequence of one or more rounds, with the expected output for each round that is graded.
- XOR is one round per example. An MNIST image fed one pixel row at a time is 28 rounds, where only the last is graded.
- **An example is a lifetime.** The memory space is reset at its start and kept across its rounds, and nothing is ever carried from one example into the next. That is the whole of the distinction between a round and an example, and it is why examples are order-independent while rounds are not. An inherited memory state does not break it: inheritance only changes what the reset resets *to*.
- The **Harness** runs one example: it resets the memory space, then runs each round in order, and hands back the output region after every round together with the ticks each took. The Evolver, which loops over the examples of its work unit, turns that into the example's **error** by calling the Verifier on the graded rounds. Results travel up uncombined.
- The level is the Harness's because an example *is* the lifetime of the memory space, and the Harness is what resets it — the only code that knows the layout being reset. A separate component for this level would have to be named for the level rather than for anything it does, which rule 3 forbids.

**Round**. One exchange: input values are written, the genome runs until it signals that its output is ready, and the output region is read. A round also ends after a configured maximum number of ticks, so a genome that never signals still produces a result: the model always answers.
- Resetting the memory space sets every wire to 0. Ready then follows two independent protocol choices: the value the ready wire is given at the start of each round (0 or 1), and the value that means "ready" (0 or 1). The **Harness** writes ready's start value at the start of every round, together with the inputs, so every round of an example opens the same way.
- The Kernel checks ready after each tick, never before the first, so every round runs at least one tick and the start value alone can never answer.
- The reference starts ready at 0 and treats 1 as ready. A genome isn't ready until some Nand drives the wire high, and a Nand reading cleared wires outputs 1, so early genomes answer after their first tick and must learn to hold ready low until their logic has settled. That makes early training faster. The other three combinations are compared against it.
- The tick maximum is a ceiling on how deep a genome's logic can be, not merely a safety valve. A signal needs one tick per layer it passes through, so a solution needing more layers than the limit allows cannot be found at all — and charging a genome for the ticks it used also charges it for depth.
- Grading happens above this level, not in it. The **Verifier** compares a graded round's produced output wires with the expected ones, bitwise, so it shows directly which wires are wrong and the Mutator and Trainer can use that as evidence. It is called by the Evolver on what the Harness hands back, and it exists only in train: turning an answer into an error is what selection needs and what the product has no use for.
- Bitwise comparison suits targets where each wire stands on its own (one wire per possible answer). For a number written in binary, a wrong high wire and a wrong low wire count the same, so such targets would need a different error.

**Tick** and **Instruction**. A tick is as defined in the README: every Nand is evaluated against the current memory space, then the results are written back in reverse Nand index order. An **instruction** is the evaluation of one Nand.
- The **Kernel** runs ticks until the ready signal or the tick limit, checking ready after each tick, and counts them. It is the only component that touches individual Nands.
- Alternative ways of scheduling Nands, such as a Nand that only runs every k-th tick, are alternative Kernels.

## The deployed program

Its only input is the model file written by the Exporter at the end of a training run, compiled into the program. Its interface is the README's input and output regions, reached through a **record** format that `main` owns.

- Records are framed. Each one begins with a single byte saying what it is: `0x00` is a **reset**, and carries nothing; `0x01` is an **input**, followed by the input region's values. The memory space never sees the header, so every possible pattern of input values stays a valid input — which is why the kind cannot be carried in-band, and why a record's length cannot carry it either. A fixed, known record length is what lets a reader treat a short read as "read more" rather than as a signal.
- `main` loops over records: on `0x01` it calls the **Harness** for one round and writes one output record; on `0x00` it has the Harness reset the memory space. An example is the span between resets. Starting a process is itself a reset, so running one process per example needs no reset records at all, and a long-lived process that serves many examples sends them.
- The **Deployment** level is one model running for as long as it is switched on. Its memory space is created once, at start-up, because there is no level above it to own one.
- Whoever embeds the model supplies raw input as bits and reads raw output bits. Because `main` returns between rounds, an embedder may choose a round's input after seeing the previous round's output.
- There is no Selector, Mutator, Trainer or Verifier, and no Config.

Evaluating several finished genomes against held-out examples is the **Driver's** work, not the product's: it builds the deployed program, feeds it held-out examples, reads the answers back through the task's target, and compares them with the labels itself. That comparison is deliberately not the Verifier's. The Verifier produces a per-example error to drive selection; the Driver produces accuracy for a report. They answer different questions and may well disagree, which is why neither is derived from the other.

## Training Data Flow

```
Experiment
┌────────────────────────────────────────────────────────────────────┐
│            ┌─────────┐      ┌───↓────┐      ┌────────┐             │
│            │ Dataset │<─────┤ Driver ├─────>│ Config │             │
│            └────┬────┘      └───┬────┘      └────────┘             │
│ Run             │               │                                  │
│ ┌───────────────↓───────────────↓────────────────────────────────┐ │
│ │               │            ┌─────┐                             │ │
│ │               │            │ Rng │                             │ │
│ │               │            └──┬──┘                             │ │
│ │ Generation    │               │                                │ │
│ │ ┌─────────────┼───────────────↓──────────────────────────────┐ │ │
│ │ │ Example     │          any component                       │ │ │
│ │ │ ┌───────────┼────────────────────────────────────────────┐ │ │ │
│ │ │ │           │         ┌─────────┐                        │ │ │ │
│ │ │ │           └────────>│ Harness │<─────────┐             │ │ │ │
│ │ │ │                     └────┬────┘          │             │ │ │ │
│ │ │ │ Round                    │               │             │ │ │ │
│ │ │ │ ┌────────────────────────↓───────────────┼───────────┐ │ │ │ │
│ │ │ │ │ Tick                   │               │           │ │ │ │ │
│ │ │ │ │ ┌──────────────────────↓───────────────┼──────┐    │ │ │ │ │
│ │ │ │ │ │  ┌────────┐      ┌────────┐      ┌───┴───┐  │    │ │ │ │ │
│ │ │ │ │ │  │ Genome ├─────>│ Kernel ├─────>│ Arena │  │    │ │ │ │ │
│ │ │ │ │ │  └───↑────┘      └────────┘      └───┬───┘  │    │ │ │ │ │
│ │ │ │ │ └──────┼───────────────────────────────┼──────┘    │ │ │ │ │
│ │ │ │ └────────┼───────────────────────────────┼───────────┘ │ │ │ │
│ │ │ └──────────┼───────────────────────────────┼─────────────┘ │ │ │
│ │ │            │                          ┌────↓─────┐         │ │ │
│ │ │            │                          │ Verifier │         │ │ │
│ │ │            │                          └────┬─────┘         │ │ │
│ │ │        ┌───┴─────┐                         │               │ │ │
│ │ │        │ Trainer │<────────────────────────┤               │ │ │
│ │ │        └─────────┘                         │               │ │ │
│ │ │                                            │               │ │ │
│ │ │        ┌─────────┐                    ┌────↓─────┐         │ │ │
│ │ │        │ Mutator │<───────────────────┤ Selector │         │ │ │
│ │ │        └────┬────┘                    └──────────┘         │ │ │
│ │ └─────────────┼──────────────────────────────────────────────┘ │ │
│ │               │                                                │ │
│ │          ┌────↓────┐                                           │ │
│ │          │ Evolver │                    any component          │ │
│ │          └────┬────┘                         ┬                 │ │
│ │          ┌────↓─────┐                    ┌───↓────┐            │ │
│ │          │ Exporter │                    │ Logger │            │ │
│ │          └────┬─────┘                    └───┬────┘            │ │
│ └───────────────┼──────────────────────────────┼─────────────────┘ │
│             ┌───↓───┐                       ┌──↓──┐                │
│             │ Model │                       │ Log │                │
│             └───↓───┘                       └──↓──┘                │
└────────────────────────────────────────────────────────────────────┘
```

The Evolver appears at the bottom because it owns Run and Generation: the boxes drawn inside those are the levels its loops repeat. The **Harness** sits inside Example and outside Round for the same reason — it owns the one and is entered at the other. The **Verifier** sits inside Generation and outside Example because it is the Evolver that calls it, on the outputs the Harness hands back. The **Config** has no arrow leaving it because it is a read-only global, read wherever it is needed rather than passed down. The **Trainer**'s arrow points back up into the Genome: it rewrites the genome between examples, which is why its examples must run in order and why it exists only in the individual-mode variant of the Evolver.

## Configurations

Six kinds of choice are configured, in decreasing scope:

- **Protocol** — train and infer must agree, or a saved genome means something different in each: how the Kernel schedules Nands, ready's start value, and which ready value means ready.
- **Training** — fixed for train, and absent from infer: whether a Trainer exists, how the Selector compares genomes, etc.
- **Inference** — fixed for infer, and absent from train: whether to parallelise, how to allocate the arena (runtime optimisations), etc.
- **Parameter** — fixed within one execution: rates, limits, maximums e.g. population size, tick timeout.
- **Execution** — choices that change runtime and memory usage but never output, e.g. thread count, the width of a word, how many examples share one, CPU or GPU implementation.
- **Task** — which problem is being solved: its source, bit order, target convention, rounds per example and graded rounds.

Training and inference configurations are collectively referred to as **algorithm** configurations.

Each kind is a prefix on the keys of the experiment file (`protocol.`, `training.`, `inference.`, `parameter.`, `execution.`, `task.`), so a key's kind is visible wherever it is written.

The **seed** is not configuration of any of these kinds. It names one Run within an experiment rather than describing the search, and where it is stated — a Driver argument, the experiment file, or a Study naming the seeds its experiments share — is undecided.

Two hashes follow from this, and neither is ever used as a path:

- The **experiment hash** covers `protocol.`, `training.`, `inference.`, `parameter.` and `task.`, and identifies a *result*. Every run directory records it, and the Driver refuses to combine runs whose hashes differ.
- The **build hash** adds the execution keys fixed at compile time, and identifies a *binary*.

`execution.` is excluded as a whole prefix, because runs that differ only in execution must give identical results, which is tested. **The seed is excluded too**: it is the one thing that varies between the runs of one experiment, so a hash that included it would identify a run rather than a result, and adding a seed later would invalidate the runs already made.

Some combinations of keys cannot be built together. They are refused in **one place**, as a compile error naming the pair, rather than by each file checking the combinations that happen to reach it: a file that guards its own is a file that can disagree with another.

The **machine** (processor, operating system, compiler version) is never configured, only recorded in every report, so runtime comparisons are valid only between runs on the same machine.

## Rules the structure follows

1. **One level, one function.** Each level of a tree is one function, containing that level's loop. The trees are therefore the program's call graph, and the top of train is a few nested loops in one function.
2. **Only components that act run loops.** Data (Dataset, Arena, Genome) is read and written, never in charge of a loop. One component may run the loops of several adjacent levels. Where a loop runs in parallel in some configurations and in order in others, that choice is made in exactly one place.
3. **Every component does something besides looping.** If the only name a proposed component can be given describes data it would track (e.g. "Clock" for a tick count), it should not exist: its loop belongs to the component that owns that data. A name must also exclude something — one that would fit three other components is not a name.
4. **Invented names are held to rule 3; established ones are not.** Where the field already has an unambiguous term for a unit, recognisability wins, because the reader arrives knowing it. "Run" is standard for one seeded search and is kept for that reason.
5. **Configuration only flows down.** It is read once from the experiment file, at Experiment, and no level below changes it.
6. **Results only flow up,** one level at a time, uncombined until they reach the Selector. Only the Selector decides how per-example results add up to a comparison between genomes. A result is therefore always per example, never per group: a measurement that covers several examples at once still reports each one separately.
7. **Memory belongs to one level and is used below it.** The Arena is a worker's scratch, created once per work unit and reused across the examples in it, reset at the start of every example and written once per tick. Deployed, there is one model, so it is created once at start-up.

## How the memory space is stored

Each wire is one `word`, and **every bit of that word holds the same value**: a wire is all zeroes or all ones, never a mixture. A Nand is then `~(a & b)` whatever the width of a word, which is what lets one Kernel serve every layout.

The general rule, which the reference and the optimisation share:

> bit *j* of a wire's word holds the value of example (*j* mod `lane_width`).

- **The reference** sets `lane_width` to 1, so every bit of a word is the same example and the statement above reduces to "all bits equal". `word` is one byte, which is the smallest form that keeps the rule and the smallest memory space, and memory traffic is what a Nand machine is limited by: evaluating a Nand is two reads at arbitrary indices, so the arrangement that keeps a genome's wires in cache beats the one that saves instructions.
- **The optimisation** sets `lane_width` to the width of a word, so one word holds one example per bit and a single pass over the Nands evaluates that many examples at once. Only what the Harness writes into the words changes; the Kernel is the same code.

Both are `execution.` keys: `execution.word_bits` and `execution.lane_width`, the latter being either 1 or the width of a word. Neither may change a result, so every result is per example from the start — a group's ticks and a group's error are arrays with one entry per example in it, not single numbers. A measurement that charged a whole group the slowest example's ticks would make the number of examples per word change the outcome, and that is exactly what must not happen.

Nothing above this section depends on which is in use. The model file stores its memory state as packed bits and is unpacked on load, so it does not depend on either.

## Open questions

- Whether evolution is best expressed as one component or as Selector + Mutator. It's possible that smarter mutation would involve knowing scores or relative performance.
