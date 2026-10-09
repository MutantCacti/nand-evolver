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

/* A word holds either one example in all its bits or one example per bit.
 * Half a word is never used, so lane_width is 1 or exactly bits_per_word.
 *
 * Each forbidden combination names the values that are forbidden, rather than
 * negating the absence of a key. A key that is missing altogether is not a
 * forbidden combination, it is an unconfigured build, and the component's own
 * #error is the one diagnostic that should fire for it. */
/* Chained rather than separate, so one mistake draws one diagnostic: a width
 * of 3 is answered by the rule it actually breaks, not by both rules at once.
 *
 * The second is the P1 limit. Lanes arrive in P2 together with the per-lane
 * tick and error accounting that makes them give identical results to the
 * reference, and that branch goes when they do. */
#if EXECUTION_LANE_WIDTH != 1 && EXECUTION_LANE_WIDTH != EXECUTION_BITS_PER_WORD
#error "execution.lane_width must be 1 or execution.bits_per_word"
#elif EXECUTION_LANE_WIDTH != 1
#error "execution.lane_width above 1 is P2; this build provides the reference only"
#endif

#endif
