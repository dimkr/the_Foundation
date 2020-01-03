/** @file math_sse.c  Vector math using SSE.

@authors Copyright (c) 2017 Jaakko Keränen <jaakko.keranen@iki.fi>

@par License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

<small>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</small>
*/

#include "the_Foundation/math.h"

void store_Mat4(const iMat4 *d, float *v) {
    _mm_storeu_ps(v,      _mm_shuffle_ps(d->col[0], d->col[0], _MM_SHUFFLE(0, 3, 2, 1)));
    _mm_storeu_ps(v + 4,  _mm_shuffle_ps(d->col[1], d->col[1], _MM_SHUFFLE(0, 3, 2, 1)));
    _mm_storeu_ps(v + 8,  _mm_shuffle_ps(d->col[2], d->col[2], _MM_SHUFFLE(0, 3, 2, 1)));
    _mm_storeu_ps(v + 12, _mm_shuffle_ps(d->col[3], d->col[3], _MM_SHUFFLE(0, 3, 2, 1)));
}

void mul_Mat4(iMat4 *d, const iMat4 *other) {
    iMat4 result;
    _Alignas(16) float dCol[4];
    for (int i = 0; i < 4; ++i) {
        _mm_store_ps(dCol, d->col[i]);
        __m128 rl =         _mm_mul_ps(other->col[0], _mm_set1_ps(dCol[1]));
        rl = _mm_add_ps(rl, _mm_mul_ps(other->col[1], _mm_set1_ps(dCol[2])));
        rl = _mm_add_ps(rl, _mm_mul_ps(other->col[2], _mm_set1_ps(dCol[3])));
        rl = _mm_add_ps(rl, _mm_mul_ps(other->col[3], _mm_set1_ps(dCol[0])));
        result.col[i] = rl;
    }
    copy_Mat4(d, &result);
}

void initRotate_Mat4(iMat4 *d, iFloat3 axis, float degrees) {
    const float ang = iMathDegreeToRadianf(degrees);
    const float c   = cosf(ang);
    const float s   = sinf(ang);
    const iFloat4 axis4 = { _mm_move_ss(normalize_F3(axis).m, _mm_setzero_ps()) };
    _Alignas(16) float ax_[4];
    _mm_store_ps(ax_, axis4.m);
    const float *ax = ax_ + 1; // x is at 1
    for (int i = 0; i < 3; ++i) {
        d->col[i] = _mm_mul_ps(_mm_mul_ps(axis4.m, _mm_set1_ps(ax[i])), _mm_set1_ps(1 - c));
    }
    d->col[0] = _mm_add_ps(d->col[0], init_F4(+c,       +ax[2]*s,   -ax[1]*s,   0).m);
    d->col[1] = _mm_add_ps(d->col[1], init_F4(-ax[2]*s, +c,         +ax[0]*s,   0).m);
    d->col[2] = _mm_add_ps(d->col[2], init_F4(+ax[1]*s, -ax[0]*s,   +c,         0).m);
    d->col[3] = init_F4(0, 0, 0, 1).m;
}

void store_Mat3(const iMat3 *d, float *v9) {
    _Alignas(16) float vals[12];
    _mm_store_ps(vals,     d->col[0]);
    _mm_store_ps(vals + 4, d->col[1]);
    _mm_store_ps(vals + 8, d->col[2]);
    v9[0] = vals[1];
    v9[1] = vals[2];
    v9[2] = vals[3];
    v9[3] = vals[5];
    v9[4] = vals[6];
    v9[5] = vals[7];
    v9[6] = vals[9];
    v9[7] = vals[10];
    v9[8] = vals[11];
}
