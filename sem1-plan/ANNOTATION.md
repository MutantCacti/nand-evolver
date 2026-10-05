# Context Annotation (draft 4)

To be appended to `ARCHITECTURE.md`. It assumes the reader has read the README and nothing else. Terms the README defines are used as it defines them: Nand, genome, memory space, wire, the input/ready/output/internal regions, tick, Nand index order. Every other term is defined here before it is used.

## What the program does

nand-evolver searches for genomes (graphs of Nands) that solve a task. A **task** is one problem to be solved: its raw data, how that data is written onto wires, how wire values are read back as answers, and which answers count as correct. The search works by evolution: it keeps a collection of genomes, measures how well each one solves the task, keeps the better ones and makes randomly changed copies of them, and repeats. A genome found this way can then be run on its own, outside the search.

So there are two programs:
- **train** performs the search (the Training Loop Tree).
- **infer** runs one finished genome on new inputs (the Inference Loop Tree).

## Reading the loop trees

Each line of a tree is a **level**. A level is one loop: it repeats the level beneath it many times (`*`).
- `( )` names the components that do the work at that level. Each is a piece of code with one job, named for what it does.
- `#` names the state (data) that the work at that level can change.

Data is never a loop owner. A name for data (Dataset, Arena) never appears in `( )`.

A level lists everything that *any* configuration of the program might change there, not only what the simplest configuration changes. For example, `# Arena` appears at Generation because one configuration lets a new genome start from a copy of its parent's memory space, even though the simplest configuration starts it empty.

## Who owns the loops

Four components own all the loops. Every other component is *called* by one of them at a fixed point.

| Owner | Loops (levels) | Calls |
|---|---|---|
| **Driver** | Study → Experiment → Run | Encoder, once per experiment |
| **Runner** | Generation → Individual | Selector and Mutator, after each generation's individuals are measured |
| **Tester** (train) / **Inferrer** (infer) | Example → Round | Kernel and Decoder every round; Trainer between examples |
| **Kernel** | Tick → Instruction | none; it evaluates Nands directly |

The levels are defined below, top to bottom, for training.

## The levels

**Study**. A set of experiments meant to be compared with each other, for example the lines of one plot. The **Driver** is a Python tool, separate from the C programs. For each experiment it builds the programs, runs them, collects their output and writes a report.
- `# Source`: the **Source** is the task's raw data, e.g. images and their **labels** (the correct answers).

**Experiment**. One fully specified search, described by one **experiment file**. That file is the **Config**: it fixes the task, the encoding, every algorithm choice and every numeric setting.
- Choices that change the program's structure are compiled in, so each combination of them is its own binary. Numeric settings are read at start-up.
- The **Encoder** runs once per experiment and converts the Source into the **Dataset**: a list of examples (defined below) expressed as wire values. It produces both the input values to write into the input region and, from the labels, the expected output values.
- Labels are encoded here because the Encoder is the only component that reads the Source. The split between the Encoder and Decoder is therefore by **direction**: the Encoder turns world values into wires, the Decoder turns wires back into world values. The consequence is that the output layout is *shared* — the Encoder writes expected values in it, the Decoder reads produced values from it — so the two must be checked against each other rather than separately. The **output layout** is the convention for representing an answer on the output wires, e.g. one wire per possible digit, with the right digit's wire set.
- The Dataset is never changed after this, so every level below can read it freely without copying it.

**Run**. One complete search with one random seed. The Driver repeats it per experiment to measure how much results vary between seeds.
- `# Rng`: the random seed. Every random choice anywhere below is computed from the seed plus its position, e.g. which example is used as the 40th example of generation 12. There is no stored random state that code shares or advances, so a run gives the same result however its work is divided between threads.
- For the same reason, examples are never shuffled into a stored order. The example to use is computed from (seed, generation, position within the generation).

**Generation**. One step of evolution, looped by the **Runner**. The **population** (the current collection of individuals, defined next) is measured; then:
- the **Selector** compares the results and chooses which individuals become parents;
- the **Mutator** makes the next population by copying parents' genomes with random changes, e.g. adding a Nand or rewiring an index.

**Individual**. One genome and its own memory space, which the code calls the **Arena**. The Runner hands each individual to a Tester, together with the range of examples to measure it on.
- Individuals never read each other's state, so they can be measured in parallel.
- How the examples are divided is decided in one place, outside the Tester: in the simplest configuration, every (individual, example) pair is a separate piece of work.
- The **Trainer** is optional. When enabled, it changes the individual's genome *during* measurement, between examples, using evidence from the results so far. Its examples must then run in order, so the Tester is handed all of them at once.

**Example**. One problem from the Dataset: a sequence of one or more rounds, with the expected output for each round that is graded.
- XOR is one round per example. An MNIST image fed one pixel row at a time is 28 rounds, where only the last is graded.
- The Arena is cleared at the start of each example and kept across its rounds. That's how a genome can remember earlier rounds.
- The **Tester** loops over the examples it was given and their rounds. It writes each round's input values into the input region, has the Kernel run the genome, and on graded rounds compares the outputs with the expected values. It records each example's result: how wrong it was (its **error**) and how many ticks it took. It passes these records up unchanged.

**Round**. One exchange: input values are written, the genome runs until it signals that its output is ready, and the output region is read. Per the README, "ready" means the genome drives wire `1+i` low. A round also ends after a configured maximum number of ticks, so a genome that never signals still produces a result.
- That maximum is a ceiling on how deep a genome's logic can be, not merely a safety valve. A signal needs one tick per layer it passes through, so a solution needing more layers than the limit allows cannot be found at all — and charging a genome for the ticks it used also charges it for depth.
- After each round, the **Decoder** reads the output region and turns it into the task's answer (e.g. which of ten digits an image shows). It also reports which output wires were wrong and how much each one matters, which the Mutator and Trainer can use to choose changes.
- Each output layout declares how its error can be attributed. Either comparing produced wires with expected wires is enough to say which are wrong and which way to move them — true when one wire means one class, or when a number is written as a count of set wires — or a wrong high-order wire and wrong low-order wires cannot be corrected independently, as in a binary number, in which case every example must be read back separately at a cost.
- The Tester tells the Decoder which rounds to skip (ungraded rounds), so the Decoder never decides what is graded. That cost is why skipping matters: an MNIST image fed as 28 rounds grades one of them.
- Because the Encoder and Decoder share the output layout, one writing it and the other reading it, they have to be checked as a pair: encoding a label and then decoding the result must return that label. Neither component can be checked on its own.

**Tick** and **Instruction**. A tick is as defined in the README: every Nand is evaluated against the current memory space, then the results are written back in reverse Nand index order. An **instruction** is the evaluation of one Nand.
- The **Kernel** runs ticks until the ready signal or the tick limit, and counts them. It is the only component that touches individual Nands.
- Alternative ways of scheduling Nands, such as a Nand that only runs every k-th tick, are alternative Kernels.

## The inference tree

infer has no search, so it has no Generation level and no Selector, Mutator, Trainer or Tester. The Runner hands the finished genome to the **Inferrer**, which runs examples from the outside world. It writes inputs, runs rounds through the Kernel, and passes every round's output through the Decoder to whoever needs it.

A finished genome is saved as a **model file**: the genome plus the identity and settings of the Encoder and Decoder it was trained with. train and infer must share the Kernel, Encoder and Decoder, or the saved genome would mean something different in each.

## Configurations

Three kinds of choice distinguish one version of the program from another, in decreasing scope:

- **Protocol** — train and infer must agree, or a saved genome means something different in each: the Encoder, the Decoder, the output layout, and how the Kernel schedules Nands.
- **Algorithm** — may differ between train and infer, but is fixed for one execution: whether a Trainer exists, how the Selector compares individuals, whether a child starts from a copy of its parent's memory space.
- **Parameter** — may vary within one execution: rates, limits, population size, the tick maximum.

Three further things vary around the program rather than within it: the **task** being solved, the **execution** (thread count, word size, target machine — these change how long a run takes and never what it produces), and the **seed**.

## Rules the structure follows

1. **Each level is one function**, so the loop tree is also the call graph, and the top of the training program is a few nested loops in one function.
2. **Only components named for an action own loops.** A component may own several adjacent levels, provided any level that is sequential under one configuration and parallel under another has exactly one place where that is declared. Data is read and written by components, never in charge of a loop.
3. **Name the owner, not the state.** If the best name available for a component is a noun for the state it tracks, then the loop belongs to whoever owns that state and the component should not exist. A component whose whole content is a loop is not a responsibility, and an awkward component name is usually evidence of a misplaced loop rather than a wording problem.
4. **Configuration flows down.** The experiment file is read once, at Experiment, and nothing below may change it.
5. **Results flow up one level at a time, unsummarised, until the Selector.** Per-example records reach the Selector intact. How they are combined into one comparison (average error, worst case, ticks used, genome size, ...) is a selection decision, so only the Selector makes it.
6. **Memory is allocated at the level that owns it and written below it.** The Arena is allocated once per individual, cleared once per example and written once per tick.
7. **A genome is charged for the Nands that do something, not for all of them.** By the README's reverse-index writeback, a Nand whose output wire is already driven by an older (lower-index) Nand never takes effect. Such Nands cost nothing and can accumulate until a later change makes one useful, so counting them against a genome's size would remove that reserve.

## How training uses the hardware

In training, each wire is stored as one 64-bit word rather than one bit, as the README's memory section describes. Bit k of every word belongs to example k, so one pass over the Nands evaluates 64 examples at once. This packing is a property of how the Tester and Kernel are written for train. It does not change any level in the tree, and infer stores one bit per wire instead.

## Open questions, settled by implementation

- Whether evolution is best expressed as one component or as Selector + Mutator.
- Whether a component always sits at the level where all of its inputs exist.
- Whether many Encoders can share one Decoder.
- Whether the shared output layout should be stated once somewhere both the Encoder and Decoder read it, given that they are now checked as a pair rather than separately.
