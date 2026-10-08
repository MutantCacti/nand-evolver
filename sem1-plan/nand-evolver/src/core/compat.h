/*
 * core/compat.h
 * Every forbidden combination of configuration keys, refused in one place.
 *
 * Included first by every translation unit, so an impossible build fails
 * before any component is compiled. One #error per incompatible pair, each
 * naming both keys: a file that guards its own combinations is a file that
 * can disagree with another about which combinations exist.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#ifndef CORE_COMPAT_H
#define CORE_COMPAT_H

#include "config.h"     /* generated per build by driver/build.py */

/* A Trainer changes a genome between examples, so a genome's examples become a
 * sequence. Only the individual-mode Evolver hands out sequences.
 *
 * Each forbidden pair names the value that is forbidden, rather than negating
 * the absence of one. A key that is missing altogether is not a forbidden
 * combination, it is an unconfigured build, and the component's own #error is
 * the one diagnostic that should fire for it. Add a line here per Trainer. */
#if defined(TRAINING_TRAINER_DELTA_ERROR) && !defined(TRAINING_EVOLVER_INDIVIDUAL)
#error "training.trainer = delta_error requires training.evolver = individual"
#endif

/* Inherited memory is the state a reset resets to, and the Evolver must keep a
 * parent's Arena alive to supply it. Nothing does that in population mode. */
#if defined(TRAINING_INHERIT_ARENA) && TRAINING_INHERIT_ARENA \
    && !defined(TRAINING_EVOLVER_INDIVIDUAL)
#error "training.inherit_arena requires training.evolver = individual"
#endif

/* A word holds either one example in all its bits or one example per bit.
 * Half a word is never used, so lane_width is 1 or exactly word_bits. */
#if EXECUTION_LANE_WIDTH != 1 && EXECUTION_LANE_WIDTH != EXECUTION_WORD_BITS
#error "execution.lane_width must be 1 or execution.word_bits"
#endif

/* P1 is the reference: one example per word. Lanes arrive in P2 together with
 * the per-lane tick and error accounting that makes them give identical results. */
#if EXECUTION_LANE_WIDTH != 1
#error "execution.lane_width above 1 is P2; this build provides the reference only"
#endif

#endif
