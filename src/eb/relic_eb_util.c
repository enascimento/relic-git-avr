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
 * Implementation of the binary elliptic curve utilities.
 *
 * @version $Id: relic_eb_util.c 1789 2014-03-02 14:50:24Z dfaranha $
 * @ingroup eb
 */

#include "relic_core.h"
#include "relic_eb.h"
#include "relic_conf.h"

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

int eb_is_infty(const eb_t p) {
	return (fb_is_zero(p->z) == 1);
}

void eb_set_infty(eb_t p) {
	fb_zero(p->x);
	fb_zero(p->y);
	fb_zero(p->z);
	p->norm = 1;
}

void eb_copy(eb_t r, const eb_t p) {
	fb_copy(r->x, p->x);
	fb_copy(r->y, p->y);
	fb_copy(r->z, p->z);
	r->norm = p->norm;
}

int eb_cmp(const eb_t p, const eb_t q) {
	if (fb_cmp(p->x, q->x) != CMP_EQ) {
		return CMP_NE;
	}

	if (fb_cmp(p->y, q->y) != CMP_EQ) {
		return CMP_NE;
	}

	if (fb_cmp(p->z, q->z) != CMP_EQ) {
		return CMP_NE;
	}

	return CMP_EQ;
}

void eb_rand(eb_t p) {
	bn_t n, k;

	bn_null(n);
	bn_null(k);

	TRY {
		bn_new(k);
		bn_new(n);

		eb_curve_get_ord(n);

		bn_rand(k, BN_POS, bn_bits(n));
		bn_mod(k, k, n);

		eb_mul_gen(p, k);
	} CATCH_ANY {
		THROW(ERR_CAUGHT);
	} FINALLY {
		bn_free(k);
		bn_free(n);
	}
}

void eb_rhs(fb_t rhs, const eb_t p) {
	fb_t t0, t1;

	fb_null(t0);
	fb_null(t1);

	TRY {
		fb_new(t0);
		fb_new(t1);

		/* t0 = x1^2. */
		fb_sqr(t0, p->x);
		/* t1 = x1^3. */
		fb_mul(t1, t0, p->x);

		/* t1 = x1^3 + a * x1^2 + b. */
		switch (eb_curve_opt_a()) {
			case OPT_ZERO:
				break;
			case OPT_ONE:
				fb_add(t1, t1, t0);
				break;
			case OPT_DIGIT:
				fb_mul_dig(t0, t0, eb_curve_get_a()[0]);
				fb_add(t1, t1, t0);
				break;
			default:
				fb_mul(t0, t0, eb_curve_get_a());
				fb_add(t1, t1, t0);
				break;
		}

		switch (eb_curve_opt_b()) {
			case OPT_ZERO:
				break;
			case OPT_ONE:
				fb_add_dig(t1, t1, 1);
				break;
			case OPT_DIGIT:
				fb_add_dig(t1, t1, eb_curve_get_b()[0]);
				break;
			default:
				fb_add(t1, t1, eb_curve_get_b());
				break;
		}

		fb_copy(rhs, t1);
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		fb_free(t0);
		fb_free(t1);
	}
}

int eb_is_valid(const eb_t p) {
	eb_t t;
	fb_t lhs;
	int r = 0;

	eb_null(t);
	fb_null(lhs);

	TRY {
		eb_new(t);
		fb_new(lhs);

		eb_norm(t, p);

		fb_mul(lhs, t->x, t->y);
		eb_rhs(t->x, t);
		fb_sqr(t->y, t->y);
		fb_add(lhs, lhs, t->y);
		r = (fb_cmp(lhs, t->x) == CMP_EQ) || eb_is_infty(p);
	}
	CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		eb_free(t);
		fb_free(lhs);
	}
	return r;
}

void eb_tab(eb_t *t, const eb_t p, int w) {
	int u;

#if defined(EB_PLAIN)
	if (!eb_curve_is_kbltz()) {
		if (w > 2) {
			eb_dbl(t[0], p);
#if defined(EB_MIXED)
			eb_norm(t[0], t[0]);
#endif
			eb_add(t[1], t[0], p);
			for (int i = 2; i < (1 << (w - 2)); i++) {
				eb_add(t[i], t[i - 1], t[0]);
			}
#if defined(EB_MIXED)
			eb_norm_sim(t + 1, (const eb_t *)t + 1, (1 << (w - 2)) - 1);
#endif
		}
		eb_copy(t[0], p);
	}
#endif /* EB_PLAIN */

#if defined(EB_KBLTZ)
	if (eb_curve_is_kbltz()) {
		u = (eb_curve_opt_a() == OPT_ZERO ? -1 : 1);

		/* Prepare the precomputation table. */
		for (int i = 0; i < 1 << (w - 2); i++) {
			eb_set_infty(t[i]);
			fb_set_dig(t[i]->z, 1);
			t[i]->norm = 1;
		}

#if defined(EB_MIXED)
		eb_norm(t[0], p);
#else
		eb_copy(t[0], p);
#endif

		switch (w) {
#if EB_DEPTH == 3 || EB_WIDTH ==  3
			case 3:
				eb_frb(t[1], t[0]);
				if (u == 1) {
					eb_sub(t[1], t[0], t[1]);
				} else {
					eb_add(t[1], t[0], t[1]);
				}
				break;
#endif
#if EB_DEPTH == 4 || EB_WIDTH ==  4
			case 4:
				eb_frb(t[3], t[0]);
				eb_frb(t[3], t[3]);

				eb_sub(t[1], t[3], p);
				eb_add(t[2], t[3], p);
				eb_frb(t[3], t[3]);

				if (u == 1) {
					eb_neg(t[3], t[3]);
				}
				eb_sub(t[3], t[3], p);
				break;
#endif
#if EB_DEPTH == 5 || EB_WIDTH ==  5
			case 5:
				eb_frb(t[3], t[0]);
				eb_frb(t[3], t[3]);

				eb_sub(t[1], t[3], p);
				eb_add(t[2], t[3], p);
				eb_frb(t[3], t[3]);

				eb_frb(t[7], t[3]);
				eb_sub(t[7], t[7], p);

				if (u == 1) {
					eb_neg(t[3], t[3]);
				}
				eb_sub(t[3], t[3], p);

				eb_frb(t[4], t[2]);
				eb_frb(t[4], t[4]);

				eb_neg(t[4], t[4]);
				eb_sub(t[5], t[4], p);
				eb_add(t[6], t[4], p);

				eb_frb(t[4], t[4]);
				if (u == -1) {
					eb_neg(t[4], t[4]);
				}
				eb_add(t[4], t[4], p);
				break;
#endif
#if EB_DEPTH == 6 || EB_WIDTH ==  6
			case 6:
				eb_frb(t[0], t[0]);
				eb_frb(t[0], t[0]);
				eb_neg(t[14], t[0]);

				eb_sub(t[13], t[14], p);
				eb_add(t[14], t[14], p);

				eb_frb(t[0], t[0]);
				eb_frb(t[8], t[0]);
				if (u == -1) {
					eb_neg(t[0], t[0]);
				}
				eb_sub(t[11], t[0], p);
				eb_add(t[12], t[0], p);

				eb_frb(t[0], t[12]);
				eb_frb(t[0], t[0]);
				eb_sub(t[1], t[0], p);
				eb_add(t[2], t[0], p);

				eb_frb(t[15], t[8]);
				if (u == -1) {
					eb_neg(t[15], t[15]);
				}
				eb_sub(t[15], t[15], p);

				eb_frb(t[0], t[13]);
				eb_frb(t[0], t[0]);
				eb_sub(t[5], t[0], p);
				eb_add(t[6], t[0], p);

				eb_sub(t[7], t[8], p);
				eb_add(t[8], t[8], p);

				eb_frb(t[0], t[0]);
				if (u == -1) {
					eb_neg(t[0], t[0]);
				}
				eb_sub(t[3], t[0], p);
				eb_add(t[4], t[0], p);

				eb_frb(t[0], t[1]);
				eb_frb(t[0], t[0]);

				eb_neg(t[9], t[0]);
				eb_sub(t[9], t[9], p);

				eb_frb(t[0], t[14]);
				eb_frb(t[0], t[0]);
				eb_add(t[10], t[0], p);

				eb_copy(t[0], p);
				break;
#endif
#if EB_DEPTH == 7 || EB_WIDTH ==  7
			/**
			 * Formulas from http://eprint.iacr.org/2012/519
			 */
			case 7:
				eb_frb(t[0], t[0]);
				eb_frb(t[0], t[0]);
				eb_sub(t[17], t[0], p);
				eb_add(t[18], t[0], p);
				eb_frb(t[0], t[0]);
				if (u == 1) {
					eb_neg(t[0], t[0]);
				}
				eb_sub(t[19], t[0], p);
				eb_add(t[20], t[0], p);

				eb_frb(t[0], t[19]);
				eb_frb(t[0], t[0]);
				eb_frb(t[11], t[0]);
				if (u == 1) {
					eb_neg(t[11], t[11]);
				}
				eb_add(t[12], t[11], p);
				eb_sub(t[11], t[11], p);
				eb_neg(t[0], t[0]);
				eb_sub(t[1], t[0], p);
				eb_add(t[2], t[0], p);

				eb_frb(t[0], t[17]);
				eb_frb(t[0], t[0]);
				eb_frb(t[0], t[0]);
				if (u == -1) {
					eb_neg(t[0], t[0]);
				}
				eb_sub(t[3], t[0], p);

				eb_frb(t[0], t[1]);
				eb_frb(t[0], t[0]);
				eb_add(t[9], t[0], p);
				eb_neg(t[9], t[9]);
				eb_frb(t[0], t[0]);
				if (u == -1) {
					eb_neg(t[0], t[0]);
				}
				eb_add(t[4], t[0], p);

				eb_frb(t[0], t[18]);
				eb_frb(t[0], t[0]);
				eb_sub(t[7], t[0], t[18]);
				eb_sub(t[25], t[0], p);
				eb_add(t[26], t[0], p);
				eb_frb(t[0], t[0]);
				if (u == -1) {
					eb_neg(t[0], t[0]);
				}
				eb_sub(t[27], t[0], p);
				eb_add(t[28], t[0], p);

				eb_frb(t[0], t[17]);
				eb_frb(t[0], t[0]);
				eb_add(t[8], t[0], t[18]);
				eb_neg(t[0], t[0]);
				eb_add(t[10], t[0], p);

				eb_frb(t[0], t[7]);
				eb_frb(t[0], t[0]);
				eb_sub(t[13], t[0], p);
				eb_frb(t[0], t[8]);
				eb_frb(t[0], t[0]);
				eb_neg(t[0], t[0]);
				eb_add(t[14], t[0], p);

				eb_frb(t[0], t[19]);
				eb_frb(t[0], t[0]);
				eb_add(t[15], t[0], t[17]);
				eb_add(t[16], t[0], t[18]);

				eb_frb(t[0], t[26]);
				eb_frb(t[0], t[0]);
				eb_neg(t[0], t[0]);
				eb_sub(t[5], t[0], p);
				eb_add(t[6], t[0], p);
				eb_add(t[23], t[0], t[17]);
				eb_add(t[24], t[0], t[18]);

				eb_frb(t[0], t[25]);
				eb_frb(t[0], t[0]);
				eb_sub(t[21], t[0], p);
				eb_add(t[22], t[0], p);

				eb_frb(t[0], t[20]);
				eb_frb(t[0], t[0]);
				eb_add(t[29], t[0], p);
				eb_neg(t[29], t[29]);

				eb_frb(t[0], t[27]);
				eb_frb(t[0], t[0]);
				eb_add(t[30], t[0], p);

				eb_frb(t[0], t[7]);
				eb_frb(t[0], t[0]);
				eb_add(t[31], t[0], t[17])

						eb_copy(t[0], p);
				break;
#endif
#if EB_DEPTH == 8 || EB_WIDTH ==  8
			/**
			 * Formulas from http://eprint.iacr.org/2012/519
			 */
			case 8:
				eb_frb(t[0], t[0]);
				eb_frb(t[0], t[0]);
				eb_neg(t[0], t[0]);
				eb_sub(t[45], t[0], p);
				eb_add(t[46], t[0], p);
				eb_frb(t[0], t[0]);
				if (u == 1) {
					eb_neg(t[0], t[0]);
				}
				eb_sub(t[43], t[0], p);
				eb_add(t[44], t[0], p);

				eb_frb(t[0], t[44]);
				eb_frb(t[0], t[0]);
				eb_sub(t[1], t[0], p);
				eb_add(t[2], t[0], p);
				eb_add(t[47], t[0], t[45]);
				eb_add(t[48], t[0], t[46]);

				eb_frb(t[0], t[46]);
				eb_frb(t[0], t[0]);
				eb_neg(t[0], t[0]);
				eb_sub(t[7], t[0], t[46]);
				eb_sub(t[8], t[0], t[45]);
				eb_frb(t[0], t[0]);
				if (u == -1) {
					eb_neg(t[0], t[0]);
				}
				eb_sub(t[3], t[0], p);

				eb_frb(t[0], t[1]);
				eb_frb(t[0], t[0]);
				eb_neg(t[0], t[0]);
				eb_add(t[55], t[0], t[45]);
				eb_add(t[56], t[0], t[46]);
				eb_sub(t[9], t[0], p);
				eb_add(t[10], t[0], p);
				eb_frb(t[0], t[0]);
				if (u == 1) {
					eb_neg(t[0], t[0]);
				}
				eb_add(t[4], t[0], p);

				eb_frb(t[0], t[47]);
				eb_frb(t[0], t[0]);
				eb_neg(t[0], t[0]);
				eb_sub(t[17], t[0], p);
				eb_add(t[18], t[0], p);

				eb_frb(t[0], t[8]);
				eb_frb(t[0], t[0]);
				eb_add(t[31], t[0], t[45]);
				eb_add(t[32], t[0], t[46]);
				eb_neg(t[0], t[0]);
				eb_sub(t[13], t[0], p);
				eb_add(t[14], t[0], p);
				eb_frb(t[0], t[0]);
				if (u == -1) {
					eb_neg(t[0], t[0]);
				}
				eb_sub(t[19], t[0], p);

				eb_frb(t[0], t[7]);
				eb_frb(t[0], t[0]);
				eb_frb(t[0], t[0]);
				if (u == -1) {
					eb_neg(t[0], t[0]);
				}
				eb_add(t[20], t[0], p);

				eb_frb(t[0], t[9]);
				eb_frb(t[0], t[0]);
				eb_sub(t[21], t[0], p);
				eb_add(t[22], t[0], p);

				eb_frb(t[0], t[2]);
				eb_frb(t[0], t[0]);
				eb_sub(t[25], t[0], p);
				eb_add(t[26], t[0], p);

				eb_frb(t[0], t[43]);
				eb_frb(t[0], t[0]);
				eb_neg(t[0], t[0]);
				eb_sub(t[33], t[0], p);
				eb_add(t[34], t[0], p);
				eb_frb(t[0], t[0]);
				if (u == -1) {
					eb_neg(t[0], t[0]);
				}
				eb_add(t[12], t[0], p);

				eb_frb(t[0], t[45]);
				eb_frb(t[0], t[0]);
				eb_sub(t[37], t[0], p);
				eb_add(t[38], t[0], p);
				eb_frb(t[0], t[0]);
				if (u == -1) {
					eb_neg(t[0], t[0]);
				}
				eb_sub(t[35], t[0], p);
				eb_add(t[36], t[0], p);

				eb_frb(t[0], t[38]);
				eb_frb(t[0], t[0]);
				eb_sub(t[41], t[0], p);
				eb_add(t[42], t[0], p);
				eb_neg(t[0], t[0]);
				eb_sub(t[39], t[0], t[46]);
				eb_sub(t[40], t[0], t[45]);

				eb_frb(t[0], t[37]);
				eb_frb(t[0], t[0]);
				eb_sub(t[5], t[0], p);
				eb_add(t[6], t[0], p);
				eb_frb(t[0], t[0]);
				if (u == -1) {
					eb_neg(t[0], t[0]);
				}
				eb_add(t[15], t[0], t[43]);
				eb_add(t[16], t[0], t[44]);
				eb_neg(t[0], t[0]);
				eb_sub(t[27], t[0], p);
				eb_add(t[28], t[0], p);

				eb_frb(t[0], t[36]);
				eb_frb(t[0], t[0]);
				eb_frb(t[0], t[0]);
				if (u == -1) {
					eb_neg(t[0], t[0]);
				}
				eb_sub(t[11], t[0], p);

				eb_frb(t[0], t[39]);
				eb_frb(t[0], t[0]);
				eb_add(t[0], t[0], p);
				eb_neg(t[49], t[0]);

				eb_frb(t[0], t[7]);
				eb_frb(t[0], t[0]);
				eb_neg(t[0], t[0]);
				eb_add(t[50], t[0], p);

				eb_frb(t[0], t[44]);
				eb_frb(t[0], t[0]);
				eb_frb(t[0], t[0]);
				if (u == 1) {
					eb_neg(t[0], t[0]);
				}
				eb_sub(t[51], t[0], p);
				eb_add(t[52], t[0], p);

				eb_frb(t[0], t[46]);
				eb_frb(t[0], t[0]);
				eb_neg(t[0], t[0]);
				eb_sub(t[53], t[0], p);
				eb_add(t[54], t[0], p);

				eb_frb(t[0], t[54]);
				eb_frb(t[0], t[0]);
				eb_add(t[23], t[0], t[45]);
				eb_add(t[24], t[0], t[46]);

				eb_frb(t[0], t[42]);
				eb_frb(t[0], t[0]);
				eb_sub(t[57], t[0], p);

				eb_frb(t[0], t[53]);
				eb_frb(t[0], t[0]);
				eb_neg(t[0], t[0]);
				eb_add(t[58], t[0], p);

				eb_frb(t[0], t[38]);
				eb_frb(t[0], t[0]);
				eb_frb(t[0], t[0]);
				if (u == -1) {
					eb_neg(t[0], t[0]);
				}
				eb_sub(t[59], t[0], p);
				eb_add(t[60], t[0], p);

				eb_frb(t[0], t[35]);
				eb_frb(t[0], t[0]);
				eb_sub(t[61], t[0], p);
				eb_add(t[62], t[0], p);

				eb_frb(t[0], t[47]);
				eb_frb(t[0], t[0]);
				eb_neg(t[0], t[0]);
				eb_add(t[63], t[0], t[45]);

				eb_frb(t[0], t[36]);
				eb_frb(t[0], t[0]);
				eb_neg(t[0], t[0]);
				eb_sub(t[29], t[0], p);
				eb_add(t[30], t[0], p);

				eb_copy(t[0], p);
				break;
#endif
		}
#if defined(EB_MIXED)
		if (w > 2) {
			eb_norm_sim(t + 1, (const eb_t *)t + 1, (1 << (w - 2)) - 1);
		}
#endif
	}
#endif /* EB_KBLTZ */
}

void eb_print(const eb_t p) {
	fb_print(p->x);
	fb_print(p->y);
	fb_print(p->z);
}

void eb_size_bin(int *size, const eb_t a, int pack) {
	eb_t t;

	eb_null(t);

	if (eb_is_infty(a)) {
		*size = 1;
		return;
	}

	TRY {
		eb_new(t);

		eb_norm(t, a);

		*size = 1 + FB_BYTES;
		if (!pack) {
			*size += FB_BYTES;
		}
	} CATCH_ANY {
		THROW(ERR_CAUGHT);
	} FINALLY {
		eb_free(t);	
	}
}

void eb_read_bin(eb_t a, const uint8_t *bin, int len) {
	if (len == 1) {
		if (bin[0] == 0) {
			eb_set_infty(a);
			return;
		} else {
			THROW(ERR_NO_BUFFER);
			return;
		}
	}

	if (len != (FB_BYTES + 1) && len != (2 * FB_BYTES + 1)) {
		THROW(ERR_NO_BUFFER);
		return;
	}

	a->norm = 1;
	fb_set_dig(a->z, 1);
	fb_read_bin(a->x, bin + 1, FB_BYTES);
	if (len == FB_BYTES + 1) {
		switch(bin[0]) {
			case 2:
				fb_zero(a->y);
				break;
			case 3:
				fb_zero(a->y);
				fb_set_bit(a->y, 0, 1);
				break;
			default:
				THROW(ERR_NO_VALID);
				break;
		}
		eb_upk(a, a);
	}

	if (len == 2 * FB_BYTES + 1) {
		if (bin[0] == 4) {
			fb_read_bin(a->y, bin + FB_BYTES + 1, FB_BYTES);
		} else {
			THROW(ERR_NO_VALID);
		}
	}
}

void eb_write_bin(uint8_t *bin, int len, const eb_t a, int pack) {
	eb_t t;

	eb_null(t);

	if (eb_is_infty(a)) {
		if (len != 1) {
			THROW(ERR_NO_BUFFER);
		} else {
			bin[0] = 0;
			return;
		}
	}

	TRY {
		eb_new(t);

		eb_norm(t, a);

		if (pack) {
			if (len != FB_BYTES + 1) {
				THROW(ERR_NO_BUFFER);	
			} else {
				eb_pck(t, t);
				bin[0] = 2 | fb_get_bit(t->y, 0);
				fb_write_bin(bin + 1, FB_BYTES, t->x);
			}
		} else {
			if (len != 2 * FB_BYTES + 1) {
				THROW(ERR_NO_BUFFER);
			} else {
				bin[0] = 4;
				fb_write_bin(bin + 1, FB_BYTES, t->x);
				fb_write_bin(bin + FB_BYTES + 1, FB_BYTES, t->y);
			}
		}
	} CATCH_ANY {
		THROW(ERR_CAUGHT);
	}
	FINALLY {
		eb_free(t);
	}
}
