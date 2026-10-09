/*
 * core/model.h
 * The model file: a run's only output, and the deployed program's only input.
 *
 * Holds the canonical genome (only (a, b) per Nand, since a canonical Nand's
 * output wire follows from its position: 1 + num_inputs + k), the input and
 * output sizes, the tick limit, and optionally the memory state a deployment
 * should start from, stored as packed bits so the file does not depend on the
 * word type.
 *
 * **The tick limit travels with the model.** Forced closure means the model
 * always answers, at the limit, with whatever its output region holds — and
 * that is how training scored it. Deploy the same genome under a different
 * limit and it is a different function, so the limit cannot be left to the
 * deployment to choose. In train it is a run-time parameter; here it is part
 * of the result.
 *
 * **This header is the format's specification.** The file crosses between the
 * C side and the Driver, so one of them has to own the layout in full rather
 * than both describing it; driver/build.py reads it and points here.
 *
 *     "NMD1"                      4 bytes, MODEL_MAGIC
 *     u32 num_inputs
 *     u32 num_outputs
 *     u32 num_internal
 *     u32 num_nands               == 1 + num_outputs + num_internal
 *     u32 tick_limit              the limit this genome was scored under
 *     u32 initial_bits            0 when there is no initial memory state
 *     num_nands * { u32 a; u32 b; }
 *     ceil(initial_bits / 8)      packed bits over the internal region, LSB-first
 *
 * Every integer is little-endian.
 *
 * **Only written here, never read here.** The Exporter writes it at the end of
 * a run; driver/build.py reads it and emits build/<name>/model.h, which is
 * what infer is compiled against. No C code parses a model file, so there is
 * no load function to go with the write.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#ifndef CORE_MODEL_H
#define CORE_MODEL_H

#include "genome.h"

#define MODEL_MAGIC   "NMD1"
#define MODEL_MAGIC_LEN 4

/* Write `genome` to `path`, with `initial` as the starting memory state
 * (packed bits over the internal region, or NULL for none). The genome must be
 * canonical: 0 on success. */
int model_write(const char * path, const Genome * genome,
                uint32_t tick_limit, const uint8_t * initial);

#endif
