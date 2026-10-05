# Context Annotation (draft 2)

To be appended to `ARCHITECTURE.md`. It assumes the reader has read the README and nothing else. Terms the README defines are used as it defines them: Nand, genome, memory space, wire, the input/ready/output/internal regions, tick, Nand index order. Every other term is defined here before it is used.

## What the program does

nand-evolver searches for genomes (graphs of Nands) that solve a task. It does this by evolution: it keeps a collection of genomes, measures how well each one solves the task, keeps the better ones and makes randomly changed copies of them, and repeats. A genome found this way can then be run on its own, outside the search.

So there are two programs:
- **train** performs the search (the Training Loop Tree).
- **infer** runs one finished genome on new inputs (the Inference Loop Tree).

## Reading the loop trees

Each line of a tree is a **level**. A level is one loop: it repeats the level beneath it many times (`*`).
- `( )` names the components that do the work at that level. Each is a piece of code with one job, named for what it does.
- `#` names the state (data) that the work at that level can change.

Data is never a loop owner. A name for data (Dataset, Arena) never appears in `( )`.

A level lists everything that *any* configuration of the program might change there, not only what the simplest configuration changes. For example, `# Arena` appears at Generation because one configuration lets a new genome start from a copy of its parent's memory space, even though the simplest configuration starts it empty.

## The levels, top to bottom (training)

**Study** (Driver). A set of experiments meant to be compared with each other, for example the lines of one plot. The **Driver** is a Python tool, separate from the C programs. For each experiment it builds the programs, runs them, collects their output and writes a report.
- `# Source`: the **Source** is the task's raw data, e.g. images and their correct labels.

**Experiment** (Encoder). One fully specified search, described by one **experiment file**. That file is the **Config**: it fixes the task, the encoding, every algorithm choice and every numeric setting.
- Choices that change the program's structure are compiled in, so each combination of them is its own binary. Numeric settings are read at start-up.
- The **Encoder** runs once per experiment and converts the Source into the **Dataset**: a list of examples (defined below) expressed as wire values. It produces both the input values to write into the input region and, from the labels, the expected output values.
- The Dataset is never changed after this, so every level below can read it freely without copying it.

**Run** (Runner). One complete search with one random seed, repeated per experiment to measure how much results vary between seeds. The **Runner** performs the search's outer loop.
- `# Rng`: the random seed. Every random choice anywhere below is computed from the seed plus its position, e.g. which example is used as the 40th example of generation 12. There is no stored random state that code shares or advances, so a run gives the same result however its work is divided between threads.
- For the same reason, examples are never shuffled into a stored order. The example to use is computed from (seed, generation, position within the generation).

**Generation** (Selector, Mutator). One step of evolution. The **population** (the current collection of individuals, defined next) is measured; then:
- the **Selector** compares the results and chooses which individuals become parents;
- the **Mutator** makes the next population by copying parents' genomes with random changes, e.g. adding a Nand or rewiring an index.

Individuals never read each other's state, so they can be measured in parallel.

**Individual** (Trainer). One genome and its own memory space, which the code calls the **Arena**.
- The **Trainer** is optional. When enabled, it changes the individual's genome *during* measurement, between examples, using evidence from the results so far.
- When it's enabled, an individual's examples must run in order. Otherwise they are independent and can run in parallel.

**Example** (Verifier, Decoder). One problem from the Dataset: a sequence of one or more rounds, with the expected output for each round that is graded.
- XOR is one round per example. An MNIST image fed one pixel row at a time is 28 rounds, where only the last is graded.
- The Arena is cleared at the start of each example and kept across its rounds. That's how a genome can remember earlier rounds.
- The **Verifier** writes each round's input values into the input region, compares outputs with expected values on graded rounds, and records the result: how wrong, and how many ticks it took.
- The **Decoder** reads the output region after each round and turns it into the task's answer (e.g. a class number). It also reports which output wires were wrong and how much each one matters, which the Mutator and Trainer can use to choose changes.
- The Verifier tells the Decoder which rounds to skip (ungraded rounds), so the Decoder never decides what is graded.

**Round**. One exchange: input values are written, the genome runs until it signals that its output is ready, and the output region is read. Per the README, "ready" means the genome drives wire `1+i` low. A round also ends after a configured maximum number of ticks, so a genome that never signals still produces a result.

**Tick** and **Instruction** (Kernel). A tick is as defined in the README: every Nand is evaluated against the current memory space, then the results are written back in reverse Nand index order. An **instruction** is the evaluation of one Nand. The **Kernel** does this work and is the only component that touches individual Nands. Alternative ways of scheduling Nands, such as a Nand that only runs every k-th tick, are alternative Kernels.

## The inference tree

infer has no search, so it has no Generation level and no Selector, Mutator, Trainer or Verifier. It runs the finished genome on examples from the outside world. The **Inferrer** writes inputs, runs rounds through the Kernel, and passes every round's output through the Decoder to whoever needs it.

A finished genome is saved as a **model file**: the genome plus the identity and settings of the Encoder and Decoder it was trained with. train and infer must share the Kernel, Encoder and Decoder, or the saved genome would mean something different in each.

## Rules the structure follows

1. **Each level is one function**, so the loop tree is also the call graph, and the top of the training program is a few nested loops in one function.
2. **Only components named for an action own loops.** A component may own several adjacent levels. Data is read and written by components, never in charge of a loop.
3. **Configuration flows down.** The experiment file is read once, at Experiment, and nothing below may change it.
4. **Results flow up one level at a time, unsummarised, until the Selector.** Per-example records reach the Selector intact. How they are combined into one comparison (average error, worst case, ticks used, genome size, ...) is a selection decision, so only the Selector makes it.
5. **Memory is allocated at the level that owns it and written below it.** The Arena is allocated once per individual, cleared once per example and written once per tick.

## How training uses the hardware

In training, each wire is stored as one 64-bit word rather than one bit, as the README's memory section describes. Bit k of every word belongs to example k, so one pass over the Nands evaluates 64 examples at once. This packing is a property of how the Verifier and Kernel are written for train. It does not change any level in the tree, and infer stores one bit per wire instead.

## Open questions, settled by implementation

- Whether evolution is best expressed as one component or as Selector + Mutator.
- Whether a component always sits at the level where all of its inputs exist.
- Whether many Encoders can share one Decoder.
