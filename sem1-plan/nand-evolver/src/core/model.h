/*
 * core/model.h
 * The model file: a run's only output, and the deployed program's only input.
 *
 * Holds the canonical genome (only (a, b) per Nand, since a canonical Nand's
 * output index is its own), the input and output sizes, and optionally the
 * memory state a deployment should start from, stored as packed bits so the
 * file does not depend on the word type.
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
int model_write(const char * path, const Genome * genome, const uint8_t * initial);

#endif
