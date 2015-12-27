
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef int16_t v4hi __attribute__ ((vector_size (8)));
typedef int32_t v2si __attribute__ ((vector_size (8)));

union v4_s16 {
	v4hi	v;
	int16_t	s16[4];
};

union v2_s32 {
	v2si	v;
	int32_t s32[2];
};

extern int32_t sat_adds32b(int32_t a, int32_t b);

/* multiply-accumulate of 8 16-bit values */
int32_t mmx_mac_unsat(int16_t *_x, int16_t *_y)
{
	register union v4_s16 *x = (union v4_s16 *)_x;
	union v4_s16 *y = (union v4_s16 *)_y;
	union v2_s32 im1, im2, r;

	/* im1.s32[0] = x->s16[0] * y->s16[0] + x->s16[1] * y->s16[1]
	 * im1.s32[1] = x->s16[2] * y->s16[2] + x->s16[3] * y->s16[3] */
	im1.v = __builtin_ia32_pmaddwd(x->v, y->v);
	/* results are saturated */

	/* im2.s32[0] = x->s16[4] * y->s16[4] + x->s16[5] * y->s16[5]
	 * im2.s32[1] = x->s16[6] * y->s16[6]+ x->s16[7] * y->s16[7] */
	im2.v = __builtin_ia32_pmaddwd((x+1)->v, (y+1)->v);
	/* results are saturated */

	/* im1.s32[0] = im1.s32[0] + im2.s32[0]
	 * im1.s32[1] = im1.s32[1] + im2.s32[1] */
	/* FIXME: overflow in addition could happen */
	im1.v = __builtin_ia32_paddd(im1.v, im2.v);

	/* FIXME: overflow in addition could happen */
	return (im1.s32[0] + im1.s32[1]) << 1;
}

/* multiply-accumulate of 8 16-bit values */
int32_t mmx_mac_sat(int16_t *_x, int16_t *_y)
{
	register union v4_s16 *x = (union v4_s16 *)_x;
	union v4_s16 *y = (union v4_s16 *)_y;
	union v2_s32 im1, im2, r;

	/* im1.s32[0] = x->s16[0] * y->s16[0] + x->s16[1] * y->s16[1]
	 * im1.s32[1] = x->s16[2] * y->s16[2] + x->s16[3] * y->s16[3] */
	im1.v = __builtin_ia32_pmaddwd(x->v, y->v);
	/* results are saturated */

	/* im2.s32[0] = x->s16[4] * y->s16[4] + x->s16[5] * y->s16[5]
	 * im2.s32[1] = x->s16[6] * y->s16[6]+ x->s16[7] * y->s16[7] */
	im2.v = __builtin_ia32_pmaddwd((x+1)->v, (y+1)->v);
	/* results are saturated */

	/* im1.s32[0] = im1.s32[0] + im2.s32[0]
	 * im1.s32[1] = im1.s32[1] + im2.s32[1] */
	im1.s32[0] = sat_adds32b(im1.s32[0], im2.s32[0]);
	im1.s32[1] = sat_adds32b(im1.s32[1], im2.s32[1]);

	/* FIXME: overflow in addition could happen */
	return sat_adds32b(im1.s32[0], im1.s32[1]) << 1;
}
