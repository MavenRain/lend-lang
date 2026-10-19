/* Known-answer words and checks for the host Pedersen and Sigma code
   (front/pedersen.h, slice S9 Part 2). Part 3 pastes the words into
   examples/zk.lang. Each word is decimal with the literal suffix u. */
#include <stdio.h>
#include <string.h>

#include "front/pedersen.h"
#include "front/u256.h"

static int checks = 0;
static int failures = 0;

static void check(const char *name, int ok) {
  checks++;
  if (!ok) {
    failures++;
    fprintf(stderr, "FAIL %s\n", name);
  }
}

static int point_eq(const G1 *a, const G1 *b) {
  U256 ax;
  U256 ay;
  U256 bx;
  U256 by;
  g1_to_words(&ax, &ay, a);
  g1_to_words(&bx, &by, b);
  return u256_cmp(&ax, &bx) == 0 && u256_cmp(&ay, &by) == 0;
}

static void scalar_mod(Fe *out, const U256 *x) {
  U256 order;
  U256 t = *x;
  unsigned i;
  for (i = 0; i < 4u; i++)
    order.limb[i] = (uint64_t)BN254_FR.w[2u * i] | ((uint64_t)BN254_FR.w[2u * i + 1u] << 32);
  while (u256_cmp(&t, &order) >= 0)
    (void)u256_sub(&t, &t, &order);
  (void)fe_from_u256(out, &t, &BN254_FR);
}

/* Recover the blind from public transcripts if Schnorr k1 equals the
   real bit-branch nonce: r = (s_bit - s1 + e_schnorr b) / e_bit. */
static int nonce_domains_separate(const G1 *h, uint64_t bit, uint64_t blind) {
  U256 v, r, raw, recovered;
  SchnorrProof sp;
  BitProof bp;
  G1 c, bc, guess;
  Fe es, eb, s, k, b, t;
  u256_from_u64(&v, bit);
  u256_from_u64(&r, blind);
  if (!pedersen_schnorr_prove(&sp, &c, &v, &r, h)
      || !pedersen_bit_prove(&bp, &bc, bit, &r, h)
      || !point_eq(&c, &bc)
      || !pedersen_schnorr_verify(&c, &sp, h)
      || !pedersen_bit_verify(&c, &bp, h)) return 0;
  pedersen_schnorr_challenge(&raw, h, &c, &sp.a);
  scalar_mod(&es, &raw);
  scalar_mod(&eb, bit == 0 ? &bp.e0 : &bp.e1);
  scalar_mod(&s, bit == 0 ? &bp.s0 : &bp.s1);
  scalar_mod(&k, &sp.s1);
  fe_from_u64(&b, bit);
  fe_mul(&t, &es, &b, &BN254_FR);
  fe_sub(&t, &k, &t, &BN254_FR);
  fe_sub(&t, &s, &t, &BN254_FR);
  fe_inv(&eb, &eb, &BN254_FR);
  fe_mul(&t, &t, &eb, &BN254_FR);
  fe_to_u256(&recovered, &t);
  return pedersen_commit(&guess, &v, &recovered, h) && !point_eq(&guess, &c);
}

static void print_word(const char *name, const U256 *w) {
  char buf[96];
  word_text(WORD_U256, w, buf, sizeof buf);
  printf("%s = %s\n", name, buf);
}

static void print_point(const char *name, const G1 *p) {
  U256 x;
  U256 y;
  char buf[128];
  g1_to_words(&x, &y, p);
  snprintf(buf, sizeof buf, "%s.x", name);
  print_word(buf, &x);
  snprintf(buf, sizeof buf, "%s.y", name);
  print_word(buf, &y);
}

static void print_scalar(const char *name, const char *field, const U256 *w) {
  char buf[128];
  snprintf(buf, sizeof buf, "%s.%s", name, field);
  print_word(buf, w);
}

static void print_schnorr(const char *name, const G1 *h, const G1 *c, const SchnorrProof *p) {
  U256 e;
  char buf[128];
  pedersen_schnorr_challenge(&e, h, c, &p->a);
  snprintf(buf, sizeof buf, "%s.A", name);
  print_point(buf, &p->a);
  print_scalar(name, "s1", &p->s1);
  print_scalar(name, "s2", &p->s2);
  print_scalar(name, "e", &e);
}

static void print_bit(const char *name, const G1 *c, const BitProof *p) {
  U256 e;
  char buf[128];
  pedersen_bit_challenge(&e, c, &p->a0, &p->a1);
  snprintf(buf, sizeof buf, "%s.C", name);
  print_point(buf, c);
  snprintf(buf, sizeof buf, "%s.A0", name);
  print_point(buf, &p->a0);
  snprintf(buf, sizeof buf, "%s.A1", name);
  print_point(buf, &p->a1);
  print_scalar(name, "e0", &p->e0);
  print_scalar(name, "e1", &p->e1);
  print_scalar(name, "s0", &p->s0);
  print_scalar(name, "s1", &p->s1);
  print_scalar(name, "e", &e);
}

/* keccak256 of 64 zero bytes, the kernel hash of two zero words. */
static int hash_zero_known(const U256 *w) {
  U256 want;
  want.limb[3] = 0xad3228b676f7d3cdULL;
  want.limb[2] = 0x4284a5443f17f196ULL;
  want.limb[1] = 0x2b36e491b30a40b2ULL;
  want.limb[0] = 0x405849e597ba5fb5ULL;
  return u256_cmp(w, &want) == 0;
}

static RangeProof range;
static RangeProof unused;

int main(void) {
  G1 g;
  G1 h;
  G1 c;
  G1 c0;
  G1 c1;
  G1 c2;
  G1 cbig;
  G1 bad;
  G1 t;
  G1 u;
  uint64_t counter = 0;
  U256 v;
  U256 r;
  U256 w;
  U256 x;
  U256 y;
  U256 zero;
  U256 one;
  U256 order;
  SchnorrProof sp;
  SchnorrProof wrong;
  BitProof b0;
  BitProof b1;
  BitProof b2;
  BitProof noncanonical;
  unsigned i;
  char name[32];

  pedersen_g(&g);
  u256_from_u64(&zero, 0);
  u256_from_u64(&one, 1);

  /* H and the hash chain. */
  check("h-derive", pedersen_h(&h, &counter));
  printf("H.counter = %llu\n", (unsigned long long)counter);
  print_point("H", &h);
  g1_to_words(&x, &y, &h);
  check("h-on-curve", g1_on_curve(&h) && !point_eq(&h, &g) && (u256_cmp(&x, &zero) != 0 || u256_cmp(&y, &zero) != 0));
  pedersen_hash2(&w, &zero, &zero);
  check("hash2-zero", hash_zero_known(&w));

  /* The commitment of v = 7 with the blind 11. */
  u256_from_u64(&v, 7);
  u256_from_u64(&r, 11);
  check("commit", pedersen_commit(&c, &v, &r, &h));
  g1_mul(&t, &g, &v);
  g1_mul(&u, &h, &r);
  g1_add(&t, &t, &u);
  g1_to_words(&x, &y, &c);
  check("commit-words", point_eq(&t, &c) && g1_from_words(&u, &x, &y) && point_eq(&u, &c));
  printf("commit.v = 7u\ncommit.blind = 11u\n");
  print_point("commit.C", &c);

  /* The Schnorr opening of C. */
  check("schnorr-prove", pedersen_schnorr_prove(&sp, &t, &v, &r, &h) && point_eq(&t, &c));
  check("schnorr-verify", pedersen_schnorr_verify(&c, &sp, &h));
  print_schnorr("schnorr", &h, &c, &sp);

  /* The bit proofs. */
  u256_from_u64(&r, 5);
  check("bit0-prove", pedersen_bit_prove(&b0, &c0, 0, &r, &h));
  check("bit0-verify", pedersen_bit_verify(&c0, &b0, &h));
  printf("bit0.blind = 5u\n");
  print_bit("bit0", &c0, &b0);
  u256_from_u64(&r, 9);
  check("bit1-prove", pedersen_bit_prove(&b1, &c1, 1, &r, &h));
  check("bit1-verify", pedersen_bit_verify(&c1, &b1, &h));
  printf("bit1.blind = 9u\n");
  print_bit("bit1", &c1, &b1);

  check("nonce-domains-bit0", nonce_domains_separate(&h, 0, 5));
  check("nonce-domains-bit1", nonce_domains_separate(&h, 1, 9));

  /* The 32-bit range proof of v = 7 with the blind 11. */
  u256_from_u64(&r, 11);
  check("range-prove", pedersen_range_prove(&range, &t, &v, &r, &h) && point_eq(&t, &c));
  check("range-verify", pedersen_range_verify(&c, &range, &h));
  check("range-open-is-schnorr", point_eq(&range.open.a, &sp.a) && u256_cmp(&range.open.s1, &sp.s1) == 0
                                   && u256_cmp(&range.open.s2, &sp.s2) == 0);
  printf("range.v = 7u\nrange.blind = 11u\n");
  print_schnorr("range.open", &h, &c, &range.open);
  for (i = 0; i < PEDERSEN_BITS; i++) {
    snprintf(name, sizeof name, "range.bit%02u", i);
    print_bit(name, &range.c[i], &range.bit[i]);
  }

  /* Negatives: a wrong s1. */
  wrong = sp;
  (void)u256_add(&wrong.s1, &wrong.s1, &one);
  check("reject-wrong-s1", !pedersen_schnorr_verify(&c, &wrong, &h));
  /* ecMul reduces these scalars, but proof encodings must be below r. */
  for (i = 0; i < 4u; i++)
    order.limb[i] = (uint64_t)BN254_FR.w[2u * i] | ((uint64_t)BN254_FR.w[2u * i + 1u] << 32);
  wrong = sp;
  (void)u256_add(&wrong.s1, &wrong.s1, &order);
  check("reject-noncanonical-schnorr-s1", !pedersen_schnorr_verify(&c, &wrong, &h));
  wrong = sp;
  (void)u256_add(&wrong.s2, &wrong.s2, &order);
  check("reject-noncanonical-schnorr-s2", !pedersen_schnorr_verify(&c, &wrong, &h));
  noncanonical = b0;
  (void)u256_add(&noncanonical.e0, &noncanonical.e0, &order);
  check("reject-noncanonical-bit-e0", !pedersen_bit_verify(&c0, &noncanonical, &h));
  noncanonical = b0;
  (void)u256_add(&noncanonical.e1, &noncanonical.e1, &order);
  check("reject-noncanonical-bit-e1", !pedersen_bit_verify(&c0, &noncanonical, &h));
  noncanonical = b0;
  (void)u256_add(&noncanonical.s0, &noncanonical.s0, &order);
  check("reject-noncanonical-bit-s0", !pedersen_bit_verify(&c0, &noncanonical, &h));
  noncanonical = b0;
  (void)u256_add(&noncanonical.s1, &noncanonical.s1, &order);
  check("reject-noncanonical-bit-s1", !pedersen_bit_verify(&c0, &noncanonical, &h));
  /* A bit value 2: the prover refuses, and the proof for b = 1 does not
     open C = 2 G + 9 H. */
  u256_from_u64(&r, 9);
  check("reject-bit-two-prover", !pedersen_bit_prove(&b2, &c2, 2, &r, &h));
  u256_from_u64(&w, 2);
  check("reject-bit-two-verify", pedersen_commit(&c2, &w, &r, &h) && !pedersen_bit_verify(&c2, &b1, &h));
  /* A point off the curve as C: (1, 3). */
  u256_from_u64(&x, 1);
  u256_from_u64(&y, 3);
  check("reject-off-curve-words", !g1_from_words(&bad, &x, &y));
  fe_from_u64(&bad.x, 1);
  fe_from_u64(&bad.y, 3);
  check("reject-off-curve-schnorr", !pedersen_schnorr_verify(&bad, &sp, &h));
  check("reject-off-curve-bit", !pedersen_bit_verify(&bad, &b0, &h));
  check("reject-off-curve-range", !pedersen_range_verify(&bad, &range, &h));
  /* A value of 2^32 + 1: the prover refuses, and the proof for 7 does not
     open its commitment. */
  u256_from_u64(&w, ((uint64_t)1 << 32) + 1u);
  u256_from_u64(&r, 11);
  check("reject-range-overflow-prover", !pedersen_range_prove(&unused, &t, &w, &r, &h));
  check("reject-range-overflow-verify", pedersen_commit(&cbig, &w, &r, &h) && !pedersen_range_verify(&cbig, &range, &h));

  /* Guest range-length regressions: the empty proof opens infinity. The
     33-bit proof appends bit1 to the valid proof of 7, so it opens
     2^32 + 7 with blind 11 + 9 * 2^32. Both must be refused as ranges. */
  check("empty-opening", pedersen_schnorr_prove(&wrong, &t, &zero, &zero, &h));
  print_point("empty.C", &t);
  print_schnorr("empty.open", &h, &t, &wrong);
  u256_from_u64(&w, ((uint64_t)1 << 32) + 7u);
  u256_from_u64(&r, ((uint64_t)9 << 32) + 11u);
  check("long-opening", pedersen_schnorr_prove(&wrong, &t, &w, &r, &h));
  print_point("long.C", &t);
  print_schnorr("long.open", &h, &t, &wrong);

  if (failures != 0) {
    fprintf(stderr, "zk: %d of %d checks failed\n", failures, checks);
    return 1;
  }
  printf("zk: %d checks passed\n", checks);
  return 0;
}
