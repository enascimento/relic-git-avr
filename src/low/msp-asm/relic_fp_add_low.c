/*
 * RELIC is an Efficient LIbrary for Cryptography
 * Copyright (C) 2007, 2008, 2009 RELIC Authors
 *
 * This file is part of RELIC. RELIC is legal property of its developers,
 * whose names are not listed here. Please refer to the COPYRIGHT file
 * for contact information.
 *
 * RELIC is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * RELIC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RELIC. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 *
 * Implementation of the low-level prime field addition and subtraction
 * functions.
 *
 * @version $Id: relic_fp_add_low.c 1538 2013-08-28 04:26:56Z dfaranha $
 * @ingroup fp
 */

#include "relic_fp.h"
#include "relic_fp_low.h"
#include "relic_core.h"

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

void fp_subc_low(dig_t *c, const dig_t *a, const dig_t *b) {
	dig_t carry = fp_subd_low(c, a, b);
	if (carry) {
		fp_addn_low(c + FP_DIGS, c + FP_DIGS, fp_prime_get());
	}
}

void fp_addc_low(dig_t *c, const dig_t *a, const dig_t *b) {
	dig_t carry = fp_addd_low(c, a, b);
	if (carry || (fp_cmpn_low(c + FP_DIGS, fp_prime_get()) != CMP_LT)) {
		fp_subn_low(c + FP_DIGS, c + FP_DIGS, fp_prime_get());
	}
}

void fp_dblm_low(dig_t *c, const dig_t *a) {
	fp_dbl(c, a);
}


void fp_subm_low(dig_t *c, const dig_t *a, const dig_t *b) {
	fp_sub(c, a, b);
}

void fp_addm_low(dig_t *c, const dig_t *a, const dig_t *b) {
	fp_add(c, a, b);
}

void fp_hlvd_low(dig_t *c, const dig_t *a) {
	dig_t carry = 0;

	if (a[0] & 1) {
		carry = fp_addn_low(c, a, fp_prime_get());
	} else {
		fp_copy(c, a);
	}

	fp_add1_low(c + FP_DIGS, a + FP_DIGS, carry);

	carry = fp_rsh1_low(c + FP_DIGS, c + FP_DIGS);
	fp_rsh1_low(c, c);
	if (carry) {
		c[FP_DIGS - 1] ^= ((dig_t)1 << (FP_DIGIT - 1));
	}
}
