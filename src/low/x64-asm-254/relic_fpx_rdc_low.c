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
 * Implementation of the low-level extension field modular reduction functions.
 *
 * @version $Id: relic_fpx_rdc_low.c 1875 2014-05-03 20:46:25Z dfaranha $
 * @ingroup fpx
 */

#include "relic_core.h"
#include "relic_fp_low.h"

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

void fp3_rdcn_low(fp3_t c, dv3_t a) {
#if FP_RDC == MONTY
	fp_rdcn_low(c[0], a[0]);
	fp_rdcn_low(c[1], a[1]);
	fp_rdcn_low(c[2], a[2]);
#else
	fp_rdc(c[0], a[0]);
	fp_rdc(c[1], a[1]);
	fp_rdc(c[2], a[2]);
#endif
}
