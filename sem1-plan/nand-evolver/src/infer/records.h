/*
 * infer/records.h
 * The record format: how an embedder talks to a deployed model.
 *
 * Two layers, and this is the outer one. The README's wire layout is between
 * the Harness and the memory space; this is between the embedder and the
 * Harness, and the memory space never sees a byte of it.
 *
 * Each record begins with one byte saying what it is. That byte is outside the
 * input values, which is the whole point: the input region must accept every
 * possible pattern, so no value in it can mean "reset", and a record's length
 * cannot mean it either — a reader only discovers that a record is short by
 * waiting, which would make a boundary depend on timing. A known length means
 * a short read is "read more", never a signal.
 *
 * Created: 2026-10-08
 *  Author: Maxence Morel Dierckx
 */
#ifndef INFER_RECORDS_H
#define INFER_RECORDS_H

#define RECORD_RESET  0x00u     /* carries nothing: start a new example */
#define RECORD_INPUT  0x01u     /* followed by RECORD_BYTES(i) input bytes */

/* Bits packed LSB-first, bit k being wire k of the region, the same packing
 * the Dataset uses. */
#define RECORD_BYTES(bits) (((size_t)(bits) + 7u) / 8u)

#endif
