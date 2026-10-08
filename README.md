# Nand-evolver

Stateful graphs of Nands evolving under selective pressure.

## Instructions

The current commit contains no ready build process as training logic is being implemented.

For a working demonstration of the Nand network, see commit `da704ce`.

## Description

We allocate a flat memory space to model computation. Nand gates are defined as three indices into the space.

The structs defining the memory space and Nand gates are in [`src/core/genome.h`](./src/core/genome.h).

## Protocol

Everything in this section describes a deployed model and the embedder that runs it. Training runs models under the same protocol, acting as their embedder; only the memory costs in the next section are specific to training.

### Memory space

The memory space is segmented by an I/O protocol:

| Bits             | Name      | Description                                                 |
| ---------------- | --------- | ----------------------------------------------------------- |
| `0`              | constant  | A reserved constant reference to 0                          |
| `[1, 1+i)`       | input     | The input space to which a model embedder writes            |
| `1+i`            | ready     | Output wire 0, which the model sets to signal that its output is ready |
| `[2+i, 2+i+m)`   | output    | The rest of the output space the model writes to            |
| `[2+i+m, n)`     | internal  | Hidden working memory of the model                          |

Where

- `i` is the size of the input in bits
- `m` is the size of the output in bits
- `n` is the total address space in bits

Bits `[0, i]` are reserved for input and thus read only to the model. Bits `[i+1, n)` are the model's own memory and read-writeable.

Nand input indices may range over `[0, n)`. Nand output indices may only range over `[i+1, n)`.

### Execution

Execution is synchronous to an internal tick rate. Nands can be evaluated in parallel and their outputs written to a buffer of size `num_nands`. Once populated, the output buffer is applied to working memory in **reverse Nand index order**.

> Since addition is append by default, prioritising smaller-index Nands in collisions prevents newly added Nands from overwriting the values of useful older Nands.

### Examples and rounds

A **round** is one exchange: the embedder writes the input space, the model runs until it sets ready or a tick limit is reached, and the embedder reads the output space. The model always answers: at the tick limit, the output space is read as it stands.

An **example** is a sequence of one or more rounds. The memory space persists between the rounds of an example, so a model can carry state from one round to the next, and is reset between examples, so it never carries state from one example to the next. A reset sets every wire to 0, or to the model's initial memory state when it has one.

### Records

A running model reads **records** from its embedder and answers each input record with one output record. Every record starts with a one-byte header:

| Header | Record | Body                  | Answer                 |
| ------ | ------ | --------------------- | ---------------------- |
| `0x00` | reset  | none                  | none                   |
| `0x01` | input  | the `i` input bits    | the `m` output bits    |

A model process starts reset. The header is never written to the memory space, so every pattern of input bits remains a valid input, and example boundaries are found by reading, never by timing.

### Words

Each wire is stored as one word of `w` bits, where `w` is fixed when the program is built (8 by default). Bit `j` of every wire's word holds the value of example `j mod L`, where `L`, the lane width, is either 1 or `w`.

- With `L = 1`, every bit of a word is equal, so a wire is `0` or all ones, and the word holds one example.
- With `L = w`, a word holds `w` examples, one per bit, so one pass over the Nands evaluates all of them.

A Nand is `~(a & b)` on whole words, which is correct at any `w` and either `L`. A saved model's initial memory state, when it has one, is stored at one bit per wire.

### Canonicalisation

During training, exported genomes are canonicalised to an optimised form.

- Nands with colliding output indices are pruned to the smallest index.
- Bits in the genome's output space that are never written to are removed, and input indices pointing to them are replaced with zero.
- Following the prior step, the output space equals the number of Nands. Nands are stripped of their output index and re-ordered in `Genome.nands` such that their index there corresponds to their output index in the memory space.

## Memory Complexity

Let `N` be the number of Nands in a genome, equal to `Genome.num_nands`.

### At Training

During training, the memory space is scaled with the number of inputs Nands might consume. The maximum size of the address space is therefore `i+1+2N` wires.

The memory space is not packed: a wire is a whole word of `w` bits rather than a single bit (see [Words](#words)). The address space costs `w(i+1+2N)` bits, and the output buffer, one word per Nand, adds `wN`.

The size of a Nand is three indices into the address space, i.e. `3log_2(i+1+2N)`. There are `N` Nands, totaling `3Nlog_2(i+1+2N)` memory.

The sum of these is `w(i+1+3N) + 3Nlog_2(i+1+2N)`.

> NB: the factor `w` on state is due to the word layout. Being constant, it does not change the complexity.

During training, memory complexity is `O(N log N)`.

Every thread evaluating a batch needs its own address space and output buffer, so the state terms multiply by thread count while the genome term does not.

### At Runtime

Canonicalised genomes are more efficient. The maximum size of their address space is `i+1+N`, with the output buffer again adding `N`.

A wire is again one word of `w` bits.

The size of canonicalised Nands is only two indices, i.e. `2log_2(i+1+N)`. There are `N` Nands, totaling `2Nlog_2(i+1+N)` memory.

The sum of these is `w(i+1+2N) + 2Nlog_2(i+1+N)`.

At runtime, memory complexity is also `O(N log N)`.

### Deployment

Asymptotic memory complexity is `O(N log N)`, but the logarithmic term is read-only at compile time.

A canonicalised genome is never written to, so it can be indexed in place. Only the address space and the output buffer are mutated, and only they consume RAM:

| Region                        | Size                 | Complexity   |
| ----------------------------- | -------------------- | ------------ |
| Nands                         | `2Nlog_2(i+1+N)`     | `Θ(N log N)` |
| address space + output buffer | `w(i+1+2N)`          | `Θ(N)`       |

On hardware with flashable ROM, RAM usage can be made linear in `N`.
