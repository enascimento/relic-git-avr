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
 * Implementation of the low-level extension field multiplication functions.
 *
 * @version $Id: relic_fpx_mul_low.c 1875 2014-05-03 20:46:25Z dfaranha $
 * @ingroup fpx
 */

#include "relic_core.h"
#include "relic_bn_low.h"
#include "relic_fp_low.h"
#include "relic_fpx_low.h"

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

void fp3_muln_low(dv3_t c, fp3_t a, fp3_t b) {
	align dig_t t0[2 * FP_DIGS], t1[2 * FP_DIGS], t2[2 * FP_DIGS], t3[2 * FP_DIGS];
	align dig_t t4[2 * FP_DIGS], t5[2 * FP_DIGS], t6[2 * FP_DIGS];

	/* Karatsuba algorithm. */

	/* t0 = a_0 * b_0, t1 = a_1 * b_1, t2 = a_2 * b_2. */
	fp_muln_low(t0, a[0], b[0]);
	fp_muln_low(t1, a[1], b[1]);
	fp_muln_low(t2, a[2], b[2]);

	/* t3 = (a_1 + a_2) * (b_1 + b_2). */
#ifdef FP_SPACE
	fp_addn_low(t3, a[1], a[2]);
	fp_addn_low(t4, b[1], b[2]);
#else
	fp_addm_low(t3, a[1], a[2]);
	fp_addm_low(t4, b[1], b[2]);
#endif
	fp_muln_low(t5, t3, t4);
	fp_addd_low(t6, t1, t2);
	fp_subc_low(t4, t5, t6);
	fp_subc_low(c[0], t0, t4);
	for (int i = -1; i > fp_prime_get_cnr(); i--) {
		fp_subc_low(c[0], c[0], t4);
	}

#ifdef FP_SPACE
	fp_addn_low(t4, a[0], a[1]);
	fp_addn_low(t5, b[0], b[1]);
#else
	fp_addm_low(t4, a[0], a[1]);
	fp_addm_low(t5, b[0], b[1]);
#endif
	fp_muln_low(t6, t4, t5);
	fp_addd_low(t4, t0, t1);
	fp_subc_low(t4, t6, t4);
	fp_subc_low(c[1], t4, t2);
	for (int i = -1; i > fp_prime_get_cnr(); i--) {
		fp_subc_low(c[1], c[1], t2);
	}

#ifdef FP_SPACE
	fp_addn_low(t5, a[0], a[2]);
	fp_addn_low(t6, b[0], b[2]);
#else
	fp_addm_low(t5, a[0], a[2]);
	fp_addm_low(t6, b[0], b[2]);
#endif
	fp_muln_low(t4, t5, t6);
	fp_addd_low(t6, t0, t2);
	fp_subc_low(t5, t4, t6);
	fp_addc_low(c[2], t5, t1);
}

void fp3_mulm_low(fp3_t c, fp3_t a, fp3_t b) {
	dv3_t t;

	dv3_null(t);

	TRY {
		dv3_new(t);
		fp3_muln_low(t, a, b);
		fp3_rdcn_low(c, t);
	} CATCH_ANY {
		THROW(ERR_CAUGHT);
	} FINALLY {
		dv3_free(t);
	}
}
