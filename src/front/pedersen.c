/* Pedersen commitments and Sigma proofs over BN254 G1 (slice S9 Part 2).
   See front/pedersen.h for the byte layouts and the transcripts. */
#include <string.h>

#include "front/pedersen.h"
#include "keccak.h"

/* The 32 big-endian bytes of X at OUT (limb 3 is the most significant), as
   eval.c word_bytes. */
static void word_bytes(const U256 *x, unsigned char *out) {
  uint32_t i;
  for (i = 0; i < 32u; i++)
    out[i] = (unsigned char)(x->limb[3u - i / 8u] >> (56u - 8u * (i % 8u)));
}

/* The word of 32 big-endian bytes at IN, as eval.c hash_words. */
static void bytes_word(U256 *out, const unsigned char *in) {
  uint32_t k;
  uint32_t j;
  for (k = 0; k < 4u; k++) {
    out->limb[3u - k] = 0;
    for (j = 0; j < 8u; j++)
      out->limb[3u - k] = (out->limb[3u - k] << 8) | in[8u * k + j];
  }
}

/* The modulus as a word. */
static void modulus_word(U256 *out, const Modulus *f) {
  unsigned i;
  for (i = 0; i < 4u; i++)
    out->limb[i] = (uint64_t)f->w[2u * i] | ((uint64_t)f->w[2u * i + 1u] << 32);
}

/* X mod F for any 256-bit X. F is above 2^253, so at most seven
   subtractions. */
static void fe_reduce(Fe *out, const U256 *x, const Modulus *f) {
  U256 m;
  U256 t = *x;
  modulus_word(&m, f);
  while (u256_cmp(&t, &m) >= 0)
    (void)u256_sub(&t, &t, &m);
  (void)fe_from_u256(out, &t, f);
}

/* X >> BITS for BITS from 1 to 63. */
static void word_shr(U256 *x, unsigned bits) {
  unsigned i;
  for (i = 0; i < 4u; i++) {
    uint64_t hi = i + 1u < 4u ? x->limb[i + 1u] << (64u - bits) : 0;
    x->limb[i] = (x->limb[i] >> bits) | hi;
  }
}

/* 1 when A is 0 or a square in Fp (Euler: A^((p - 1) / 2) = 1). */
static int fp_is_square(const Fe *a) {
  U256 e;
  Fe one;
  Fe zero;
  Fe t;
  fe_from_u64(&zero, 0);
  if (fe_eq(a, &zero))
    return 1;
  modulus_word(&e, &BN254_FP);
  word_shr(&e, 1u);
  fe_from_u64(&one, 1);
  fe_pow(&t, a, &e, &BN254_FP);
  return fe_eq(&t, &one);
}

/* The smaller square root of a square A in Fp. p = 3 mod 4, so one root is
   A^((p + 1) / 4) and the other is p minus it. */
static void fp_sqrt_small(Fe *out, const Fe *a) {
  U256 e;
  U256 one;
  Fe y0;
  Fe y1;
  modulus_word(&e, &BN254_FP);
  word_shr(&e, 2u);
  u256_from_u64(&one, 1);
  (void)u256_add(&e, &e, &one);
  fe_pow(&y0, a, &e, &BN254_FP);
  fe_neg(&y1, &y0, &BN254_FP);
  *out = fe_lt(&y0, &y1) ? y0 : y1;
}

static int g1_eq(const G1 *a, const G1 *b) {
  return fe_eq(&a->x, &b->x) && fe_eq(&a->y, &b->y);
}

void pedersen_g(G1 *out) {
  fe_from_u64(&out->x, 1);
  fe_from_u64(&out->y, 2);
}

int pedersen_h(G1 *out, uint64_t *counter) {
  unsigned char data[PEDERSEN_TAG_LEN + 32u];
  unsigned char digest[32];
  uint64_t n;
  memcpy(data, PEDERSEN_TAG, PEDERSEN_TAG_LEN);
  for (n = 0; n < 256u; n++) {
    U256 w;
    Fe x;
    Fe rhs;
    Fe three;
    u256_from_u64(&w, n);
    word_bytes(&w, data + PEDERSEN_TAG_LEN);
    keccak256(data, sizeof data, digest);
    bytes_word(&w, digest);
    fe_reduce(&x, &w, &BN254_FP);
    fe_from_u64(&three, 3);
    fe_mul(&rhs, &x, &x, &BN254_FP);
    fe_mul(&rhs, &rhs, &x, &BN254_FP);
    fe_add(&rhs, &rhs, &three, &BN254_FP);
    if (fp_is_square(&rhs)) {
      out->x = x;
      fp_sqrt_small(&out->y, &rhs);
      *counter = n;
      return 1;
    }
  }
  return 0;
}

void pedersen_hash2(U256 *out, const U256 *a, const U256 *b) {
  unsigned char data[64];
  unsigned char digest[32];
  word_bytes(a, data);
  word_bytes(b, data + 32);
  keccak256(data, sizeof data, digest);
  bytes_word(out, digest);
}

void pedersen_chain(U256 *out, const U256 *words, size_t count) {
  U256 h;
  size_t i;
  pedersen_hash2(&h, &words[0], &words[1]);
  for (i = 2; i < count; i++)
    pedersen_hash2(&h, &h, &words[i]);
  *out = h;
}

typedef enum {
  NONCE_SCHNORR = 1,
  NONCE_BIT = 2,
  NONCE_RANGE = 3
} NonceDomain;

/* Separate protocols even when they prove the same commitment opening. */
static void nonce(Fe *out, const U256 *secret, const G1 *c, NonceDomain domain, uint64_t index) {
  unsigned char data[160];
  unsigned char digest[32];
  U256 x;
  U256 y;
  U256 w;
  g1_to_words(&x, &y, c);
  u256_from_u64(&w, (uint64_t)domain);
  word_bytes(secret, data);
  word_bytes(&x, data + 32);
  word_bytes(&y, data + 64);
  word_bytes(&w, data + 96);
  u256_from_u64(&w, index);
  word_bytes(&w, data + 128);
  keccak256(data, sizeof data, digest);
  bytes_word(&w, digest);
  fe_reduce(out, &w, &BN254_FR);
}

/* K times P for a scalar K of Fr. */
static void mul_fe(G1 *out, const G1 *p, const Fe *k) {
  U256 w;
  G1 t;
  fe_to_u256(&w, k);
  g1_mul(&t, p, &w);
  *out = t;
}

/* V G + R H for V and R in Fr. */
static void commit_fe(G1 *out, const Fe *v, const Fe *r, const G1 *h) {
  G1 g;
  G1 vg;
  G1 rh;
  pedersen_g(&g);
  mul_fe(&vg, &g, v);
  mul_fe(&rh, h, r);
  g1_add(out, &vg, &rh);
}

int pedersen_commit(G1 *out, const U256 *v, const U256 *r, const G1 *h) {
  Fe fv;
  Fe fr;
  if (!fe_from_u256(&fv, v, &BN254_FR) || !fe_from_u256(&fr, r, &BN254_FR))
    return 0;
  commit_fe(out, &fv, &fr, h);
  return 1;
}

void pedersen_schnorr_challenge(U256 *e, const G1 *h, const G1 *c, const G1 *a) {
  U256 words[8];
  G1 g;
  pedersen_g(&g);
  g1_to_words(&words[0], &words[1], &g);
  g1_to_words(&words[2], &words[3], h);
  g1_to_words(&words[4], &words[5], c);
  g1_to_words(&words[6], &words[7], a);
  pedersen_chain(e, words, 8u);
}

int pedersen_schnorr_prove(SchnorrProof *out, G1 *c, const U256 *v, const U256 *r, const G1 *h) {
  Fe fv;
  Fe fr;
  Fe k1;
  Fe k2;
  Fe e;
  Fe t;
  U256 raw;
  if (!fe_from_u256(&fv, v, &BN254_FR) || !fe_from_u256(&fr, r, &BN254_FR))
    return 0;
  commit_fe(c, &fv, &fr, h);
  nonce(&k1, r, c, NONCE_SCHNORR, 0);
  nonce(&k2, r, c, NONCE_SCHNORR, 1);
  commit_fe(&out->a, &k1, &k2, h);
  pedersen_schnorr_challenge(&raw, h, c, &out->a);
  fe_reduce(&e, &raw, &BN254_FR);
  fe_mul(&t, &e, &fv, &BN254_FR);
  fe_add(&t, &t, &k1, &BN254_FR);
  fe_to_u256(&out->s1, &t);
  fe_mul(&t, &e, &fr, &BN254_FR);
  fe_add(&t, &t, &k2, &BN254_FR);
  fe_to_u256(&out->s2, &t);
  return 1;
}

/* 1 when S is a canonical scalar, below r. */
static int scalar_ok(const U256 *s) {
  Fe t;
  return fe_from_u256(&t, s, &BN254_FR);
}

/* 1 when LHS equals A + E C, with E raw as in 0x07. */
static int dl_equation(const G1 *lhs, const G1 *a, const U256 *e, const G1 *c) {
  G1 rhs;
  g1_mul(&rhs, c, e);
  g1_add(&rhs, &rhs, a);
  return g1_eq(lhs, &rhs);
}

int pedersen_schnorr_verify(const G1 *c, const SchnorrProof *proof, const G1 *h) {
  G1 g;
  G1 lhs;
  G1 t;
  U256 e;
  if (!g1_on_curve(c) || !g1_on_curve(h) || !g1_on_curve(&proof->a))
    return 0;
  if (!scalar_ok(&proof->s1) || !scalar_ok(&proof->s2))
    return 0;
  pedersen_g(&g);
  pedersen_schnorr_challenge(&e, h, c, &proof->a);
  g1_mul(&lhs, &g, &proof->s1);
  g1_mul(&t, h, &proof->s2);
  g1_add(&lhs, &lhs, &t);
  return dl_equation(&lhs, &proof->a, &e, c);
}

void pedersen_bit_challenge(U256 *e, const G1 *c, const G1 *a0, const G1 *a1) {
  U256 words[6];
  g1_to_words(&words[0], &words[1], c);
  g1_to_words(&words[2], &words[3], a0);
  g1_to_words(&words[4], &words[5], a1);
  pedersen_chain(e, words, 6u);
}

/* C minus J G, the point whose log base H the branch J proves. */
static void branch_point(G1 *out, const G1 *c, unsigned j) {
  G1 g;
  if (j == 0) {
    *out = *c;
    return;
  }
  pedersen_g(&g);
  fe_neg(&g.y, &g.y, &BN254_FP);
  g1_add(out, c, &g);
}

int pedersen_bit_prove(BitProof *out, G1 *c, uint64_t b, const U256 *r, const G1 *h) {
  Fe fb;
  Fe fr;
  Fe k;
  Fe ej;
  Fe sj;
  Fe e;
  Fe ei;
  Fe si;
  Fe t;
  U256 raw;
  G1 cj;
  G1 u;
  G1 w;
  G1 *ai;
  G1 *aj;
  unsigned i;
  unsigned j;
  if (b > 1u || !fe_from_u256(&fr, r, &BN254_FR))
    return 0;
  i = (unsigned)b;
  j = 1u - i;
  fe_from_u64(&fb, b);
  commit_fe(c, &fb, &fr, h);
  nonce(&k, r, c, NONCE_BIT, 0);
  nonce(&ej, r, c, NONCE_BIT, 1);
  nonce(&sj, r, c, NONCE_BIT, 2);
  ai = i == 0 ? &out->a0 : &out->a1;
  aj = i == 0 ? &out->a1 : &out->a0;
  /* The false branch J is a simulation: A_j = s_j H minus e_j (C minus j G). */
  branch_point(&cj, c, j);
  mul_fe(&u, h, &sj);
  mul_fe(&w, &cj, &ej);
  fe_neg(&w.y, &w.y, &BN254_FP);
  g1_add(aj, &u, &w);
  /* The true branch I: A_i = k H, e_i = e minus e_j, s_i = k + e_i r. */
  mul_fe(ai, h, &k);
  pedersen_bit_challenge(&raw, c, &out->a0, &out->a1);
  fe_reduce(&e, &raw, &BN254_FR);
  fe_sub(&ei, &e, &ej, &BN254_FR);
  fe_mul(&t, &ei, &fr, &BN254_FR);
  fe_add(&si, &t, &k, &BN254_FR);
  fe_to_u256(i == 0 ? &out->e0 : &out->e1, &ei);
  fe_to_u256(i == 0 ? &out->e1 : &out->e0, &ej);
  fe_to_u256(i == 0 ? &out->s0 : &out->s1, &si);
  fe_to_u256(i == 0 ? &out->s1 : &out->s0, &sj);
  return 1;
}

int pedersen_bit_verify(const G1 *c, const BitProof *proof, const G1 *h) {
  Fe e0;
  Fe e1;
  Fe e;
  Fe sum;
  U256 raw;
  G1 cj;
  G1 lhs;
  if (!g1_on_curve(c) || !g1_on_curve(h) || !g1_on_curve(&proof->a0) || !g1_on_curve(&proof->a1))
    return 0;
  if (!fe_from_u256(&e0, &proof->e0, &BN254_FR) || !fe_from_u256(&e1, &proof->e1, &BN254_FR))
    return 0;
  if (!scalar_ok(&proof->s0) || !scalar_ok(&proof->s1))
    return 0;
  pedersen_bit_challenge(&raw, c, &proof->a0, &proof->a1);
  fe_reduce(&e, &raw, &BN254_FR);
  fe_add(&sum, &e0, &e1, &BN254_FR);
  if (!fe_eq(&sum, &e))
    return 0;
  g1_mul(&lhs, h, &proof->s0);
  if (!dl_equation(&lhs, &proof->a0, &proof->e0, c))
    return 0;
  branch_point(&cj, c, 1u);
  g1_mul(&lhs, h, &proof->s1);
  return dl_equation(&lhs, &proof->a1, &proof->e1, &cj);
}

int pedersen_range_prove(RangeProof *out, G1 *c, const U256 *v, const U256 *r, const G1 *h) {
  U256 limit;
  U256 blind[PEDERSEN_BITS];
  Fe fv;
  Fe fr;
  Fe acc;
  Fe t;
  Fe two;
  Fe weight;
  Fe inv;
  unsigned i;
  u256_from_u64(&limit, (uint64_t)1 << PEDERSEN_BITS);
  if (u256_cmp(v, &limit) >= 0 || !fe_from_u256(&fr, r, &BN254_FR))
    return 0;
  (void)fe_from_u256(&fv, v, &BN254_FR);
  commit_fe(c, &fv, &fr, h);
  /* 31 blinds from the nonce derivation. The sum of 2^i r_i must equal r, so
     the last blind is solved: r_31 = (r minus the sum) / 2^31. */
  fe_from_u64(&acc, 0);
  fe_from_u64(&weight, 1);
  fe_from_u64(&two, 2);
  for (i = 0; i + 1u < PEDERSEN_BITS; i++) {
    Fe ri;
    nonce(&ri, r, c, NONCE_RANGE, i);
    fe_to_u256(&blind[i], &ri);
    fe_mul(&t, &ri, &weight, &BN254_FR);
    fe_add(&acc, &acc, &t, &BN254_FR);
    fe_mul(&weight, &weight, &two, &BN254_FR);
  }
  fe_sub(&t, &fr, &acc, &BN254_FR);
  fe_inv(&inv, &weight, &BN254_FR);
  fe_mul(&t, &t, &inv, &BN254_FR);
  fe_to_u256(&blind[PEDERSEN_BITS - 1u], &t);
  for (i = 0; i < PEDERSEN_BITS; i++) {
    if (!pedersen_bit_prove(&out->bit[i], &out->c[i], (v->limb[0] >> i) & 1u, &blind[i], h))
      return 0;
  }
  return pedersen_schnorr_prove(&out->open, c, v, r, h);
}

int pedersen_range_verify(const G1 *c, const RangeProof *proof, const G1 *h) {
  G1 acc;
  unsigned i;
  if (!pedersen_schnorr_verify(c, &proof->open, h))
    return 0;
  for (i = 0; i < PEDERSEN_BITS; i++) {
    if (!pedersen_bit_verify(&proof->c[i], &proof->bit[i], h))
      return 0;
  }
  /* The sum of 2^i C_i by Horner from the top bit. */
  acc = proof->c[PEDERSEN_BITS - 1u];
  for (i = PEDERSEN_BITS - 1u; i > 0; i--) {
    g1_double(&acc, &acc);
    g1_add(&acc, &acc, &proof->c[i - 1u]);
  }
  return g1_eq(&acc, c);
}
