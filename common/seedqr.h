#ifndef DICEROLL_SEEDQR_H
#define DICEROLL_SEEDQR_H

#include <stddef.h>
#include "mnemonic_state.h"

/* Standard SeedQR: four decimal digits per zero-based word index. */
enum { SEEDQR_TEXT_CAPACITY = 24 * 4 + 1 };
int seedqr_encode( const MnemonicState *state, char *text, size_t capacity );

#endif
