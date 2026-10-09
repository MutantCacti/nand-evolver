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

#endif
