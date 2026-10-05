# Architecture

A summary of the planned nand-evolver data and responsibility structure.

It assumes the reader has read the README and nothing else. Terms the README defines are used as it defines them: Nand, genome, memory space, wire, the input/ready/output/internal regions, tick, Nand index order. Every other term is defined here before it is used.

## What the program does

nand-evolver searches for genomes (graphs of Nands) that solve a task. A **task** is one problem to be solved: its raw data, how that data is written onto wires, how wire values are read back as answers, and which answers count as correct. The search works by evolution: it keeps a collection of genomes, measures how well each one solves the task, keeps the better ones and makes randomly changed copies of them, and repeats. A genome found this way can then be run on its own, outside the search.

So there are two programs:
- **train** performs the search.
- **infer** is the product: one finished genome, running on input from the world.

## Reading the loop trees

Each line of a tree is a **level**. A level is one loop: it repeats the level beneath it many times (`*`).
- `( )` names the components that do the work at that level. Each is a piece of code with one job, named for what it does.
- `#` names the state (data) that the work at that level can change.

Data is never a loop owner. A name for data (Dataset, Arena) never appears in `( )`.

A level's `( )` lists every component that acts there: the owner, whose loop at that level repeats the level beneath, and any components it calls. An owner therefore appears at each level whose loop it runs.

A level lists everything that *any* configuration of the program might change there, not only what the simplest configuration changes. For example, `# Arena` appears at Generation because one configuration lets a new genome start from a copy of its parent's memory space, even though the simplest configuration starts it empty.

## Loop Trees

### Training

```
Study (Driver)                                          # Source
└─* Experiment (Driver, Encoder)                        # Config, Dataset
    └─* Run (Evolver)                                   # Rng
        └─* Generation (Evolver, Selector, Mutator)     # Genome, Arena
            └─* Individual (Harness, Trainer)           # Genome, Arena
                └─* Example (Harness)                   # Arena
                    └─* Round (Kernel, Decoder, Verifier)   # Arena
                        └─* Tick (Kernel)               # Arena
                            └─* Instruction             # Arena
```

### Deployment

```
Deployment (Harness)                                    # Arena, Model
└─* Example (Harness)                                   # Arena
    └─* Round (Encoder, Kernel, Decoder)                # Arena
        └─* Tick (Kernel)                               # Arena
            └─* Instruction                             # Arena
```

The deployed program has no Study, Experiment, Run or Individual level, because an embedder shipping a model does not loop over experiments, seeds or candidate genomes. Those levels are the workbench's, not the product's.

## Who owns the loops

Four components own all the loops. Every other component is *called* by one of them at a fixed point.

| Owner | Sits at | Repeats | Calls |
|---|---|---|---|
| **Driver** | Study, Experiment | Experiments, Runs | Encoder, once per experiment |
| **Evolver** | Run, Generation | Generations, Individuals | Selector then Mutator, after a generation's individuals are measured |
| **Harness** | Individual, Example (train); Deployment, Example (deployed) | Examples, Rounds | Kernel and Decoder every round; Verifier on graded rounds and Trainer between examples (train); Encoder every round (deployed) |
| **Kernel** | Round, Tick | Ticks, Instructions | none; it evaluates Nands directly |

The **Harness is the same component in both programs.** It repeats examples and their rounds, driving the protocol the README defines: write the input region, run until the genome signals ready, read the output region. In train it is fed from the Dataset and calls the Verifier and Trainer; deployed, it is fed live by the Encoder and calls neither.

## The levels

Top to bottom, for training.

**Study**. A set of experiments meant to be compared with each other, for example the lines of one plot. The **Driver** is a Python tool, separate from the C programs. For each experiment it builds the programs, runs them, collects their output and writes a report.
- `# Source`: the **Source** is the task's raw data, e.g. images and their **labels** (the correct answers).

**Experiment**. One fully specified search, described by one **experiment file**. That file is the **Config**: it fixes the task, the encoding, every algorithm choice and every numeric setting. It is a workbench artifact; the deployed program never sees one.
- Choices that change the program's structure are compiled in, so each combination of them is its own binary. Numeric settings are read at start-up.
- The **Encoder** runs once per experiment and converts the Source into the **Dataset**: a list of examples (defined below) expressed as wire values. It produces both the input values to write into the input region and, from the labels, the expected output values.
- Labels are encoded here because the Encoder is the only component that reads the Source. The split between the Encoder and Decoder is therefore by **direction**: the Encoder turns world values into wires, the Decoder turns wires back into world values. The consequence is that the output layout is *shared* — the Encoder writes expected values in it, the Decoder reads produced values from it — so the two must be checked against each other rather than separately. The **output layout** is the convention for representing an answer on the output wires, e.g. one wire per possible digit, with the right digit's wire set.
- The Dataset is never changed after this, so every level below can read it freely without copying it.

**Run**. One complete search from one seed. The Driver repeats it per experiment to measure how much results vary between seeds. Only train has runs.
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

**Round**. One exchange: input values are written, the genome runs until it signals that its output is ready, and the output region is read. Per the README, "ready" means the genome drives wire `1+i` low. A round also ends after a configured maximum number of ticks, so a genome that never signals still produces a result.
- That maximum is a ceiling on how deep a genome's logic can be, not merely a safety valve. A signal needs one tick per layer it passes through, so a solution needing more layers than the limit allows cannot be found at all — and charging a genome for the ticks it used also charges it for depth.
- After each round, the **Decoder** reads the output region and turns it into the task's answer (e.g. which of ten digits an image shows). It also reports which output wires were wrong and how much each one matters, which the Mutator and Trainer can use to choose changes.
- On graded rounds the **Verifier** compares the produced wires with the expected ones. It exists only in train: it is what turns an answer into an error, and nothing in the product needs that.
- Each output layout declares how its error can be attributed. Either comparing produced wires with expected wires is enough to say which are wrong and which way to move them — true when one wire means one possible answer, or when a number is written as a count of set wires — or a wrong high-order wire and wrong low-order wires cannot be corrected independently, as in a binary number, in which case every example must be read back separately at a cost.
- The Harness tells the Decoder which rounds to skip (ungraded rounds), so the Decoder never decides what is graded. That cost is why skipping matters: an MNIST image fed as 28 rounds grades one of them.
- Because the Encoder and Decoder share the output layout, one writing it and the other reading it, they have to be checked as a pair: encoding a label and then decoding the result must return that label. Neither component can be checked on its own.

**Tick** and **Instruction**. A tick is as defined in the README: every Nand is evaluated against the current memory space, then the results are written back in reverse Nand index order. An **instruction** is the evaluation of one Nand.
- The **Kernel** runs ticks until the ready signal or the tick limit, and counts them. It is the only component that touches individual Nands.
- Alternative ways of scheduling Nands, such as a Nand that only runs every k-th tick, are alternative Kernels.

## The deployed program

Its only input is a **model file**: the genome, plus the identity and settings of the Encoder and Decoder it was trained with. Without those the genome's bits cannot be interpreted.

- The **Deployment** level is one model running for as long as it is switched on. Its Arena is created once, at start-up, because there is no Individual level to own it.
- The **Encoder** runs per round here, on live input, rather than once over a stored Dataset.
- There is no Selector, Mutator, Trainer or Verifier, and no Config.

train and infer share the Harness, Kernel, Encoder and Decoder. Every part of the path a genome experiences is therefore the same in both, which is what makes a trained genome mean the same thing when deployed.

Evaluating several finished genomes against held-out examples is the **Driver's** work, not the product's: it builds the deployed program, feeds it examples, and compares the answers with labels itself. That comparison is deliberately not the Verifier's. The Verifier produces a per-example error to drive selection; the Driver produces accuracy for a report. They answer different questions and may well disagree, which is why neither is derived from the other.

## Training Data Flow

```
Experiment
┌──────────────────────────────────────────────────────────────────────────────────────┐
│                        ┌─────────┐                                                   │
│                        │ Encoder │                                                   │
│                        └────┬────┘                                                   │
│ Run                         │                                                        │
│ ┌───────────────────────────↓──────────────────────────────────────────────────────┐ │
│ │                      ┌─────────┐                ┌─────┐                          │ │
│ │                      │ Dataset │                │ Rng │                          │ │
│ │                      └────┬────┘                └──┬──┘                          │ │
│ │ Generation                │                        │                             │ │
│ │ ┌─────────────────────────↓────────────────────────↓───────────────────────────┐ │ │
│ │ │ Individual                                                                   │ │ │
│ │ │ ┌──────────────────────────────────────────────────────────────────────────┐ │ │ │
│ │ │ │ Example                                                                  │ │ │ │
│ │ │ │ ┌──────────────────────────────────────────────────────────────────────┐ │ │ │ │
│ │ │ │ │ Round                                                                │ │ │ │ │
│ │ │ │ │ ┌────────────────────────────────────────────────────┐               │ │ │ │ │
│ │ │ │ │ │ Tick                                               │               │ │ │ │ │
│ │ │ │ │ │ ┌───────────────────────────────────┐              │               │ │ │ │ │
│ │ │ │ │ │ │ ┌────────┐  ┌────────┐  ┌───────┐ │  ┌─────────┐ │  ┌──────────┐ │ │ │ │ │
│ │ │ │ │ │ │ │ Genome ├─>│ Kernel ├─>│ Arena ├─┼─>│ Decoder ├─┼─>│ Verifier │ │ │ │ │ │
│ │ │ │ │ │ │ └────────┘  └────────┘  └───────┘ │  └────┬────┘ │  └────┬─────┘ │ │ │ │ │
│ │ │ │ │ │ └─────↑─────────────────────────────┘       │      │       │       │ │ │ │ │
│ │ │ │ │ └───────┼─────────────────────────────────────┼──────┘       │       │ │ │ │ │
│ │ │ │ └─────────┼─────────────────────────────────────┼──────────────┼───────┘ │ │ │ │
│ │ │ │           │      ┌─────────┐                    │              │         │ │ │ │
│ │ │ │           ├──────┤ Trainer │<───────────────────┴──────────────┤         │ │ │ │
│ │ │ │           │      └─────────┘                                   │         │ │ │ │
│ │ │ └───────────┼────────────────────────────────────────────────────┼─────────┘ │ │ │
│ │ │             │      ┌─────────┐              ┌──────────┐         │           │ │ │
│ │ │             └──────┤ Mutator │<─────────────┤ Selector │<────────┘           │ │ │
│ │ │                    └─────────┘              └──────────┘                     │ │ │
│ │ └──────────────────────────────────────────────────────────────────────────────┘ │ │
│ └──────────────────────────────────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────────────────────────────────┘
```

## Configurations

Three kinds of choice distinguish one version of the program from another, in decreasing scope:

- **Protocol** — train and infer must agree, or a saved genome means something different in each: the Encoder, the Decoder, the output layout, and how the Kernel schedules Nands.
- **Algorithm** — may differ between train and infer, but is fixed for one execution: whether a Trainer exists, how the Selector compares individuals, whether a child starts from a copy of its parent's memory space.
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
- Whether many Encoders can share one Decoder.
- Whether the shared output layout should be stated once somewhere both the Encoder and Decoder read it, given that they are checked as a pair rather than separately.
