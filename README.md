# Nand-evolver

Stateful graphs of Nands evolving under selective pressure.

## Instructions

The current commit contains no ready build process as training logic is being implemented.

For a working demonstration of the Nand network, see commit `da704ce`.

## Description

We allocate a flat memory space to model computation. Nand gates are defined as three indices into the space.

The structs defining the memory space and Nand gates are in [`src/core/genome.h`](./src/core/genome.h).

### Protocol

The memory space is segmented by an I/O protocol:

| Bits             | Name      | Description                                                 |
| ---------------- | --------- | ----------------------------------------------------------- |
| `0`              | constant  | A reserved constant reference to 0                          |
| `[1, 1+i)`       | input     | The input space to which a model embedder writes            |
| `1+i`            | ready     | Output wire 0 which the model must set active-low to output |
| `[2+i, 2+i+m)`   | output    | The rest of the output space the model writes to            |
| `[2+i+m, n)`     | internal  | Hidden working memory of the model                          |

Where

- `i` is the size of the input in bits
- `m` is the size of the output in bits
- `n` is the total address space in bits

Bits `[0, i]` are reserved for input and thus enforced read only at training. Bits `[i+1, n)` are the model's own memory and read-writeable.

Nand input indices may range over `[0, n)`. Nand output indices may only range over `[i+1, n)`.

### Execution

Execution is synchronous to an internal tick rate. Nands can be evaluated in parallel and their outputs written to a buffer of size `num_nands`. Once populated, the output buffer is applied to working memory in **reverse Nand index order**.

> Since addition is append by default, prioritising smaller-index Nands in collisions prevents newly added Nands from overwriting the values of useful older Nands.

### Canonicalisation

During training, exported genomes are canonicalised to an optimised form.

- Nands with colliding output indices are pruned to the smallest index.
- Bits in the genome's output space that are never written to are removed, and input indices pointing to them are replaced with zero.
- Following the prior step, the output space equals the number of Nands. Nands are stripped of their output index and re-ordered in `Genome.nands` such that their index there corresponds to their output index in the memory space.

### Memory Complexity

Let `N` be the number of Nands in a genome, equal to `Genome.num_nands`.

#### At Training

During training, the memory space is scaled with the number of inputs Nands might consume. The maximum size of the address space is therefore `i+1+2N` wires.

Training does not pack the memory space, so a wire is a whole word (currently `uint64_t`) rather than a single bit. The address space costs `64(i+1+2N)` bits, and the output buffer, one word per Nand, adds `64N`.

The size of a Nand is three indices into the address space, i.e. `3log_2(i+1+2N)`. There are `N` Nands, totaling `3Nlog_2(i+1+2N)` memory.

The sum of these is `64(i+1+3N) + 3Nlog_2(i+1+2N)`.

> NB: 64x on state is due to the training layout. Using a full word per wire allows 64 training examples to be evaluated per operation, parallelising training runs.
> Being constant, it does not change the complexity, but it is the reason training memory is dominated by state where runtime memory is dominated by the genome.

During training, memory complexity is `O(N log N)`.

Every thread evaluating a batch needs its own address space and output buffer, so the state terms multiply by thread count while the genome term does not.

#### At Runtime

Canonicalised genomes are more efficient. The maximum size of their address space is `i+1+N`, with the output buffer again adding `N`.

Runtime uses a bit-packed layout, so a wire is one bit.

The size of canonicalised Nands is only two indices, i.e. `2log_2(i+1+N)`. There are `N` Nands, totaling `2Nlog_2(i+1+N)` memory.

The sum of these is `i+1+2N + 2Nlog_2(i+1+N)`.

At runtime, memory complexity is also `O(N log N)`.

#### Deployment

Asymptotic memory complexity is `O(N log N)`, but the logarithmic term is read-only at compile time.

A canonicalised genome is never written to, so it can be indexed in place. Only the address space and the output buffer are mutated, and only they consume RAM:

| Region                        | Size                 | Complexity   |
| ----------------------------- | -------------------- | ------------ |
| Nands                         | `2Nlog_2(i+1+N)`     | `Θ(N log N)` |
| address space + output buffer | `i+1+2N`             | `Θ(N)`       |

On hardware with flashable ROM, RAM usage can be made linear in `N`.
