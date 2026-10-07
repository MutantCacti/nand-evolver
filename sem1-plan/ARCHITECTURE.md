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

Each line of a tree is a **level**, and each level is one loop. A level's `( )` lists every component that acts there: the level's **owner**, whose loop repeats the level beneath, and the components it calls. An owner therefore appears at every level whose loop it runs.

A level's `#` lists everything that *any* configuration of the program might create or change there, not only what the simplest configuration does. For example, `# Arena` appears at Generation because one configuration lets a new genome start from a copy of its parent's memory space, even though the simplest configuration starts it empty.

### Training

```
Study (Driver)                                              # Source
└─* Experiment (Driver)                                     # Config, Dataset
    └─* Run (Evolver)                                       # Rng
        └─* Generation (Evolver, Selector, Mutator)         # Genome, Arena
            └─* Individual (Harness, Trainer)               # Genome, Arena
                └─* Example (Harness)                       # Arena
                    └─* Round (Kernel, Verifier)            # Arena
                        └─* Tick (Kernel)                   # Arena
                            └─* Instruction                 # Arena
```

### Deployment

```
Deployment (Harness)                        # Arena, Model
└─* Example (Harness)                       # Arena
    └─* Round (Kernel)                      # Arena
        └─* Tick (Kernel)                   # Arena
            └─* Instruction                 # Arena
```

The deployed program has no Study, Experiment, Run or Individual level, because an embedder shipping a model does not loop over experiments, seeds or candidate genomes. Those levels are the workbench's, not the product's.

## Who owns the loops

Four components own all the loops. Every other component is *called* by one of them at a fixed point.

| Owner | Sits at | Repeats | Calls |
|---|---|---|---|
| **Driver** | Study, Experiment | Experiments, Runs | builds the Dataset once per experiment |
| **Evolver** | Run, Generation | Generations, Individuals | Selector then Mutator, after a generation's individuals are measured |
| **Harness** | Individual, Example (train); Deployment, Example (deployed) | Examples, Rounds | Kernel every round; Verifier on graded rounds and Trainer between examples (train) |
| **Kernel** | Round, Tick | Ticks, Instructions | none; it evaluates Nands directly |

The **Harness is the same component in both programs.** It repeats examples and their rounds, driving the protocol the README defines: write the input region, run until the genome signals ready, read the output region. In train it is fed from the Dataset and calls the Verifier and Trainer; deployed, it is fed input records by whoever embeds the model and calls neither.

## The levels

Top to bottom, for training.

**Study**. A set of experiments meant to be compared with each other, for example the lines of one plot. The **Driver** is a Python tool, separate from the C programs. For each experiment it builds the programs, runs them, collects their output and writes a report.
- `# Source`: the **Source** is the task's raw data, e.g. images and their **labels** (the correct answers).

**Experiment**. One fully specified search, described by one **experiment file**. That file is the **Config**: it fixes the task, every algorithm choice and every numeric setting. It is a workbench artifact; the deployed program never sees one.
- Choices that change the program's structure are compiled in, so each combination of them is its own binary. Numeric settings are read at start-up.
- The Driver converts the Source into the **Dataset**: a list of examples (defined below) expressed as wire values. Raw input data is flattened into bits and chunked into rounds; it is never encoded into an invented representation, so a genome must learn whatever encoding of its raw input suits it.
- The one output-side convention is the **target**: how a label becomes the expected values of the output wires, e.g. one wire per possible digit with the right digit's wire set, or a label's raw bits. The task names its target, and the Driver uses the same convention in reverse to read answers back.
- The Dataset is never changed after this, so every level below can read it freely without copying it.

**Run**. One complete search from one seed. The Driver repeats it per experiment to measure how much results vary between seeds. Only train has runs. The **Evolver** carries out a run, generation after generation. When the run ends, it writes the best genome it found as a **model file**: the genome, its input and output sizes, and optionally the memory state it should start from. That file is training's only output and the deployed program's only input.
- `# Rng`: the random seed. Every random choice anywhere below is computed from the seed plus its position, e.g. which example is used as the 40th example of generation 12. There is no stored random state that code shares or advances, so a run gives the same result however its work is divided between threads.
- For the same reason, examples are never shuffled into a stored order. The example to use is computed from (seed, generation, position within the generation).

**Generation**. One step of evolution. The **population** (the current collection of individuals, defined next) is measured; then:
- the **Selector** compares the results and chooses which individuals become parents;
- the **Mutator** makes the next population by copying parents' genomes with random changes, e.g. adding a Nand or rewiring an index.

**Individual**. One genome and its own memory space, which the code calls the **Arena**. The Evolver hands each individual to the Harness, together with the range of examples to measure it on.
- Individuals never read each other's state, so they can be measured in parallel.
- How the examples are divided is decided in one place, outside the Harness: in the simplest configuration, every (individual, example) pair is a separate piece of work.
- The **Trainer** is optional. When enabled, it changes the individual's genome *during* measurement, between examples, using evidence from the results so far. Its examples must then run in order, so the Harness is handed all of them at once.

**Example**. One problem from the Dataset: a sequence of one or more rounds, with the expected output for each round that is graded.
- XOR is one round per example. An MNIST image fed one pixel row at a time is 28 rounds, where only the last is graded.
- The Arena is cleared at the start of each example and kept across its rounds. That's how a genome can remember earlier rounds.
- The **Harness** loops over the examples it was given and their rounds, writing each round's input values into the input region and having the Kernel run the genome. It records each example's result — how wrong it was (its **error**) and how many ticks it took — and passes those records up unchanged.

**Round**. One exchange: input values are written, the genome runs until it signals that its output is ready, and the output region is read. A round also ends after a configured maximum number of ticks, so a genome that never signals still produces a result: the model always answers.
- Clearing the memory space sets every wire to 0. Ready then follows two independent protocol choices: the value the ready wire is given at the start of each round (0 or 1), and the value that means "ready" (0 or 1). The Harness writes ready's start value at the start of every round, together with the inputs, so every round of an example opens the same way.
- The Kernel checks ready after each tick, never before the first, so every round runs at least one tick and the start value alone can never answer.
- The reference starts ready at 0 and treats 1 as ready. A genome isn't ready until some Nand drives the wire high, and a Nand reading cleared wires outputs 1, so early genomes answer after their first tick and must learn to hold ready low until their logic has settled. That makes early training faster. The other three combinations are compared against it.
- The tick maximum is a ceiling on how deep a genome's logic can be, not merely a safety valve. A signal needs one tick per layer it passes through, so a solution needing more layers than the limit allows cannot be found at all — and charging a genome for the ticks it used also charges it for depth.
- On graded rounds the **Verifier** compares the produced output wires with the expected ones. The comparison is bitwise, so it shows directly which wires are wrong, and the Mutator and Trainer can use that as evidence to choose changes. It exists only in train: it is what turns an answer into an error, and nothing in the product needs that.
- Bitwise comparison suits targets where each wire stands on its own (one wire per possible answer). For a number written in binary, a wrong high wire and a wrong low wire count the same, so such targets would need a different error.

**Tick** and **Instruction**. A tick is as defined in the README: every Nand is evaluated against the current memory space, then the results are written back in reverse Nand index order. An **instruction** is the evaluation of one Nand.
- The **Kernel** runs ticks until the ready signal or the tick limit, checking ready after each tick, and counts them. It is the only component that touches individual Nands.
- Alternative ways of scheduling Nands, such as a Nand that only runs every k-th tick, are alternative Kernels.

## The deployed program

Its only input is the model file written by the Evolver at the end of a training run, compiled into the program. Its interface is the README's input and output regions: for each input record it reads, it writes one output record.

- The **Deployment** level is one model running for as long as it is switched on. Its Arena is created once, at start-up, because there is no Individual level to own it.
- Whoever embeds the model supplies raw input as bits and reads raw output bits. By default one process runs one example, so starting a new process clears the memory space.
- There is no Selector, Mutator, Trainer or Verifier, and no Config.

train and infer share the Harness, the Kernel and the genome format. Every part of the path a genome experiences is therefore the same in both, which is what makes a trained genome mean the same thing when deployed.

Evaluating several finished genomes against held-out examples is the **Driver's** work, not the product's: it builds the deployed program, feeds it held-out examples, reads the answers back through the task's target, and compares them with the labels itself. That comparison is deliberately not the Verifier's. The Verifier produces a per-example error to drive selection; the Driver produces accuracy for a report. They answer different questions and may well disagree, which is why neither is derived from the other.

## Training Data Flow

```
Experiment
┌────────────────────────────────────────────────────────────────────────────────────────────┐
│                   ┌─────────┐     ┌────────┐      ┌────────┐                               │
│                   │ Dataset │<────┤ Driver ├─────>│ Config │                               │
│                   └────┬────┘     └───┬────┘      └───┬────┘                               │
│ Run                    │              │               └─────────────┐                      │
│ ┌──────────────────────↓──────────────↓─────────────────────────────↓────────────────────┐ │
│ │                      │           ┌─────┐                          │                    │ │
│ │                      │           │ Rng │                          │                    │ │
│ │                      │           └──┬──┘                          │                    │ │
│ │ Generation           │              │                             │                    │ │
│ │ ┌────────────────────┼──────────────↓─────────────────────────────┼──────────────────┐ │ │
│ │ │ Individual         │                                            │                  │ │ │
│ │ │ ┌──────────────────┼────────────────────────────────────────────┼────────────────┐ │ │ │
│ │ │ │                  │            ┌─────────┐                     │                │ │ │ │
│ │ │ │                  └───────────>│ Harness │                     │                │ │ │ │
│ │ │ │                               └────┬────┘                     │                │ │ │ │
│ │ │ │ Example                            │                          │                │ │ │ │
│ │ │ │ ┌──────────────────────────────────┼──────────────────────────┼──────────────┐ │ │ │ │
│ │ │ │ │ Round                            │                          │              │ │ │ │ │
│ │ │ │ │ ┌────────────────────────────────┼──────────────────────────┼────────────┐ │ │ │ │ │
│ │ │ │ │ │ Tick                           │                          │            │ │ │ │ │ │
│ │ │ │ │ │ ┌──────────────────────────────┼────────┐                 │            │ │ │ │ │ │
│ │ │ │ │ │ │ ┌────────────────────────────┼──────┐ │                 │            │ │ │ │ │ │
│ │ │ │ │ │ │ │                            │      │ │                 │            │ │ │ │ │ │
│ │ │ │ │ │ │ │ ┌────────┐  ┌────────┐  ┌──↓────┐ │ │   ┌──────────┐  │            │ │ │ │ │ │
│ │ │ │ │ │ │ │ │ Genome ├─>│ Kernel ├─>│ Arena ├─┼─┼──>│ Verifier │  │            │ │ │ │ │ │
│ │ │ │ │ │ │ │ └────────┘  └────────┘  └───────┘ │ │   └────┬─────┘  │            │ │ │ │ │ │
│ │ │ │ │ │ │ └─────↑─────────────────────────────┘ │        │        │            │ │ │ │ │ │
│ │ │ │ │ │ └───────┼───────────────────────────────┘        │        │            │ │ │ │ │ │
│ │ │ │ │ └─────────┼────────────────────────────────────────┼────────┼────────────┘ │ │ │ │ │
│ │ │ │ └───────────┼────────────────────────────────────────┼────────┼──────────────┘ │ │ │ │
│ │ │ │             │      ┌─────────┐                       │        │                │ │ │ │
│ │ │ │             ├──────┤ Trainer │<──────────────────────┤        │                │ │ │ │
│ │ │ │             │      └─────────┘                       │        │                │ │ │ │
│ │ │ └─────────────┼────────────────────────────────────────┼────────┼────────────────┘ │ │ │
│ │ │               │      ┌─────────┐        ┌──────────┐   │        │                  │ │ │
│ │ │               └──────┤ Mutator │<───────┤ Selector │<──┘        │                  │ │ │
│ │ │                      └─────────┘        └────┬─────┘            │                  │ │ │
│ │ └──────────────────────────────────────────────┼──────────────────┼──────────────────┘ │ │
│ │                                           ┌────↓────┐             │                    │ │
│ │                                           │ Evolver │<────────────┘                    │ │
│ │                                           └────┬────┘                                  │ │
│ └────────────────────────────────────────────────┼───────────────────────────────────────┘ │
│                                           ┌──────↓─────┐                                   │
│                                           │ Model file │                                   │
│                                           └────────────┘                                   │
└────────────────────────────────────────────────────────────────────────────────────────────┘
```

## Configurations

Three kinds of choice distinguish one version of the program from another, in decreasing scope:

- **Protocol** — train and infer must agree, or a saved genome means something different in each: how the Kernel schedules Nands, ready's start value, and which ready value means ready.
- **Algorithm** — fixed for one execution of train, and absent from infer: whether a Trainer exists, how the Selector compares individuals, whether a child starts from a copy of its parent's memory space.
- **Parameter** — may vary within one execution: rates, limits, population size, the tick maximum.

Three further things vary between experiments, around the program rather than within it: the **task** being solved, the **execution** (thread count, word size, target machine — these change how long a run takes and never what it produces), and the **replicate** (the seed).

Each of those has a unit one level up in the tree, or none at all: the task varies between Experiments, the replicate between Runs, and the execution between nothing, because it never changes what a run produces.

## Rules the structure follows

1. **One level, one function.** Each level of a tree is one function, containing that level's loop. The trees are therefore the program's call graph, and the top of train is a few nested loops in one function.
2. **Only components that act run loops.** Data (Dataset, Arena, Genome) is read and written, never in charge of a loop. One component may run the loops of several adjacent levels. Where a loop runs in parallel in some configurations and in order in others, that choice is made in exactly one place.
3. **Every component does something besides looping.** If the only name a proposed component can be given describes data it would track (e.g. "Clock" for a tick count), it should not exist: its loop belongs to the component that owns that data. A name must also exclude something — one that would fit three other components is not a name.
4. **Invented names are held to rule 3; established ones are not.** Where the field already has an unambiguous term for a unit, recognisability wins, because the reader arrives knowing it. "Run" is standard for one seeded search and is kept for that reason.
5. **Configuration only flows down.** It is read once from the experiment file, at Experiment, and no level below changes it.
6. **Results only flow up,** one level at a time, uncombined until they reach the Selector. Only the Selector decides how per-example results add up to a comparison between individuals.
7. **Memory belongs to one level and is used below it.** The Arena is created once per individual, cleared once per example and written once per tick. Deployed, there is one individual, so it is created once at start-up.

## How training uses the hardware

In training, each wire is stored as one 64-bit word rather than one bit, as the README's memory section describes. Bit k of every word belongs to example k, so one pass over the Nands evaluates 64 examples at once. This packing is a property of how the Harness and Kernel are written for train. It does not change any level in the tree, and infer stores one bit per wire instead.

## Open questions, settled by implementation

- Whether evolution is best expressed as one component or as Selector + Mutator.
- Whether a component always sits at the level where all of its inputs exist.
