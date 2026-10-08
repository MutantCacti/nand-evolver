# SYN

Synthetic plan building with independent contexts.

## Goals

Answer key questions relating to the future of nand-evolver:

1. What is the correct abstractable data flow hierarchy that enables SRP handling of important evolutionary tasks across different levels of the loop?

2. How should top-level configuration in the build and in the binary CLI manage variations on program design at various levels, including mutually exclusive or paired variations?

3. How can encapsulation be maximised without loss of customisable data flow, including the all-important customisability of the verification step required to properly encode a task's expected output format through the scoring function?

## Context

Two agents: DELTA and THREAD (in wall-time age order). Both involved in important planning conversations, now both with good ideas and relevant context about the nand-evolver project.

The current status of the codebase is **under construction**. It should be considered a reference.

Discussions with project superviser Prof. Chris Jefferson have resulted in the following important considerations:

- Variations should be composable at will; project quality depends on the ability to compare stepped improvements in the program and the way certain optimisations interact.
- Global configuration or context variables are a recommended pattern under the strict rule that they remain read-only and top level.
- Data flow only should determine the layout of a program architecture: a strong data flow allows the complete abstraction of details like parallelisation/SIMD across boundaries between components.

## Task Description

Two staged phases of planning.

### Phase 1: Clarification

1A. **Discussion**. DELTA and THREAD communicate using `wire`. Both have had very different conversations with `mutant` and should aim to understand what the other has learnt and learn what the other understands.

1B. **Investigation**. DELTA and THREAD produce a priority series of A/B decisions based on existing uncertainties in the intended plan. STOP. `mutant` responds with decisions, clarifications, or follow-up questions. Discussion continues until `mutant` concludes stage 1B.

1C. **Review**. `mutant` provides a simple summary of the current architecture model. DELTA and THREAD append a drafted, reviewed and polished context annotation, aiming to match the degree of brevity while ensuring comprehensiveness for future agents. STOP.

### Phase 2: Draft

2A. **Files**. DELTA and ATLAS use the Phase 1 specification to construct a **filesystem plan**. A filesystem plan is a `tree`-esque document outlining the specific files and directory structure of a codebase. For a strong reference, see lines 433:641 of /home/mutant/mitespotter-api/planning.md. STOP. Iterative review with `mutant` until they conclude stage 2A.

2B. **Functions**. DELTA and ATLAS create the planned filesystem in a new directory, including non-boilerplate files. Each such file is stubbed with their **boundary functions**, that is, the functions that reach from or into other files, and which in the eventual program will contain the only reference to all other (private) functions in the file. STOP. Iterative review with `mutant` until they conclude stage 2B.

2C. **Review**. `mutant` spawns a third agent (designation: DESTUB) to perform a **blind wiring test** of the planning directory. This test involves writing fake, fast-running functions that model the expected data flow in order to subject real smoke tests to desired behaviours, including config variation and parallelisation. The rule is: DESTUB implements the program (including boilerplate e.g. Makefiles), while DELTA and THREAD run tests. Once DESTUB has a running version working, DELTA and THREAD produce a list of tests to run, then STOP. `mutant` confirms and they are run. Iterative review until all tests pass to all agents' definition of success.

## Practical Direction

Unless otherwise specified or requested, all files should be created in repository `MutantCacti/nand-evolver` on branch `sem1-plan`. DELTA and THREAD are on different devices and must push/pull to receive each other's changes, they must plan accordingly. Do not touch `main`, especially not in remote.

DELTA and DESTUB can find the working dir at /home/mutant/nand-evolver/sem1-plan
THREAD and ATLAS can find the working dir at /home/mutant/proj/nand-evolver/sem1-plan

Commits should be title-only, with no body and no Co-Authored-By. Follow existing git practices.

In any uncertainty, always feel comfortable to STOP and ask `mutant` for direction.

Have fun.

