/*
 * RELIC is an Efficient LIbrary for Cryptography
 * Copyright (C) 2007-2014 RELIC Authors
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
 * Implementation of utilities in extensions defined over prime fields.
 *
 * @version $Id: relic_fpx_util.c 1867 2014-05-03 19:50:27Z dfaranha $
 * @ingroup fpx
 */

#include "relic_core.h"
#include "relic_fpx_low.h"

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

void fp2_copy(fp2_t c, fp2_t a) {
	fp_copy(c[0], a[0]);
	fp_copy(c[1], a[1]);
}

void fp2_zero(fp2_t a) {
	fp_zero(a[0]);
	fp_zero(a[1]);
}

int fp2_is_zero(fp2_t a) {
	return fp_is_zero(a[0]) && fp_is_zero(a[1]);
}

void fp2_rand(fp2_t a) {
	fp_rand(a[0]);
	fp_rand(a[1]);
}

void fp2_print(fp2_t a) {
	fp_print(a[0]);
	fp_print(a[1]);
}

void fp2_size_bin(int *size, const fp2_t a) {
	*size = FP_BYTES * 2;
}

void fp2_read_bin(fp2_t a, const uint8_t *bin, int len) {
	if (len != FP_BYTES * 2) {
		THROW(ERR_NO_BUFFER);
	}
	fp_read_bin(a[0], bin, FP_BYTES);
	fp_read_bin(a[1], bin + FP_BYTES, FP_BYTES);
}

void fp2_write_bin(uint8_t *bin, int len, const fp2_t a) {
	if (len != FP_BYTES * 2) {
		THROW(ERR_NO_BUFFER);
	}
	fp_write_bin(bin, FP_BYTES, a[0]);
	fp_write_bin(bin + FP_BYTES, FP_BYTES, a[1]);
}

void fp2_set_dig(fp2_t a, dig_t b) {
	fp_set_dig(a[0], b);
	fp_zero(a[1]);
}


void fp3_copy(fp3_t c, fp3_t a) {
	fp_copy(c[0], a[0]);
	fp_copy(c[1], a[1]);
	fp_copy(c[2], a[2]);
}

void fp3_zero(fp3_t a) {
	fp_zero(a[0]);
	fp_zero(a[1]);
	fp_zero(a[2]);
}

int fp3_is_zero(fp3_t a) {
	return fp_is_zero(a[0]) && fp_is_zero(a[1]) && fp_is_zero(a[2]);
}

void fp3_rand(fp3_t a) {
	fp_rand(a[0]);
	fp_rand(a[1]);
	fp_rand(a[2]);
}

void fp3_print(fp3_t a) {
	fp_print(a[0]);
	fp_print(a[1]);
	fp_print(a[2]);
}


void fp6_copy(fp6_t c, fp6_t a) {
	fp2_copy(c[0], a[0]);
	fp2_copy(c[1], a[1]);
	fp2_copy(c[2], a[2]);
}

void fp6_zero(fp6_t a) {
	fp2_zero(a[0]);
	fp2_zero(a[1]);
	fp2_zero(a[2]);
}

int fp6_is_zero(fp6_t a) {
	return fp2_is_zero(a[0]) && fp2_is_zero(a[1]) && fp2_is_zero(a[2]);
}

void fp6_rand(fp6_t a) {
	fp2_rand(a[0]);
	fp2_rand(a[1]);
	fp2_rand(a[2]);
}

void fp6_print(fp6_t a) {
	fp2_print(a[0]);
	fp2_print(a[1]);
	fp2_print(a[2]);
}

void fp12_copy(fp12_t c, fp12_t a) {
	fp6_copy(c[0], a[0]);
	fp6_copy(c[1], a[1]);
}

void fp12_zero(fp12_t a) {
	fp6_zero(a[0]);
	fp6_zero(a[1]);
}

int fp12_is_zero(fp12_t a) {
	return (fp6_is_zero(a[0]) && fp6_is_zero(a[1]));
}

void fp12_rand(fp12_t a) {
	fp6_rand(a[0]);
	fp6_rand(a[1]);
}

void fp12_print(fp12_t a) {
	fp6_print(a[0]);
	fp6_print(a[1]);
}

void fp12_set_dig(fp12_t a, dig_t b) {
	fp12_zero(a);
	fp_set_dig(a[0][0][0], b);
}

void fp18_copy(fp18_t c, fp18_t a) {
	fp6_copy(c[0], a[0]);
	fp6_copy(c[1], a[1]);
	fp6_copy(c[2], a[2]);
}

void fp18_zero(fp18_t a) {
	fp6_zero(a[0]);
	fp6_zero(a[1]);
	fp6_zero(a[2]);
}

int fp18_is_zero(fp18_t a) {
	return (fp6_is_zero(a[0]) && fp6_is_zero(a[1]) && fp6_is_zero(a[2]));
}

void fp18_rand(fp18_t a) {
	fp6_rand(a[0]);
	fp6_rand(a[1]);
	fp6_rand(a[2]);
}

void fp18_print(fp18_t a) {
	fp6_print(a[0]);
	fp6_print(a[1]);
	fp6_print(a[2]);
}

void fp18_set_dig(fp18_t a, dig_t b) {
	fp18_zero(a);
	fp_set_dig(a[0][0][0], b);
}
