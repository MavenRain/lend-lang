/* The S12 ring signer for test/zk.sh and the zk.lang review rows: a linkable
   ring signature (LSAG) over the poll point Q of examples/zk.lang. It is a
   test tool:  the nonces come from the secret and the message, not from a
   random source.

   Modes (decimal words in, decimal words out):
     key X                          prints px py, the key X G.
     point TAG POLL                 prints k qx qy. k is the first k >= 0 where
                                    qx = zkBaseX is less than p and qx^3 + 3
                                    is a square mod p. qy = (qx^3 + 3)^((p+1)/4)
                                    mod p (BN254 p = 3 mod 4).
     sign TAG POLL K QY CH J X0 .. [bad]
                                    prints ix iy c0 s0 s1 s2 s3. The ring is
                                    X0 G, X1 G, .. in the slot order of the
                                    contract (1 to 4 keys, the order after the
                                    swap-removes). The signer is at slot J.
                                    s_i = 0 for an empty slot i. bad adds 1 to
                                    s0.

   The words are the words of zk.lang. hash is the kernel 2-word hash
   (pedersen_hash2:  keccak256 over 64 bytes, the same as OP_HASH in
   src/front/eval.c). zkMsg = hash (hash TAG POLL) CH and zkBaseX =
   hash (hash (hash TAG POLL) K) 0. The chain step is
   c' = H(H(H(H(m, Lx), Ly), Rx), Ry) with L = s G + c P and R = s Q + c I.
   The raw chain word is the ecMul scalar (0x07 reduces mod r). The signer
   reduces the words mod r for the Fr arithmetic. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "front/u256.h"
#include "front/bn254.h"
#include "front/pedersen.h"

#define KMAX 4

static U256 rmod;

static void dec(U256 *out, const char *s) {
  u256_from_u64(out, 0);
  for (; *s >= '0' && *s <= '9'; s++) u256_scale_add(out, 10, (uint32_t)(*s - '0'));
}

static void put(const U256 *x) {
  char b[128];
  word_text(WORD_U256, x, b, sizeof b);
  printf(" %s", b);
}

static void red(Fe *out, const U256 *raw) {
  U256 t = *raw;
  while (u256_cmp(&t, &rmod) >= 0) u256_sub(&t, &t, &rmod);
  fe_from_u256(out, &t, &BN254_FR);
}

static void chainpt(U256 *h, const G1 *p) {
  U256 x, y, t, u;
  g1_to_words(&x, &y, p);
  pedersen_hash2(&t, h, &x);
  pedersen_hash2(&u, &t, &y);
  *h = u;
}

static void step(U256 *c, const U256 *m, const G1 *g, const G1 *q, const G1 *img, const G1 *p, const U256 *s) {
  G1 a, b, l, r;
  U256 h = *m;
  g1_mul(&a, g, s);
  g1_mul(&b, p, c);
  g1_add(&l, &a, &b);
  g1_mul(&a, q, s);
  g1_mul(&b, img, c);
  g1_add(&r, &a, &b);
  chainpt(&h, &l);
  chainpt(&h, &r);
  *c = h;
}

/* zkBaseX:  hash (hash (hash TAG POLL) K) 0. */
static void basex(U256 *out, const U256 *tag, const U256 *poll, const U256 *k) {
  U256 a, b, zero;
  u256_from_u64(&zero, 0);
  pedersen_hash2(&a, tag, poll);
  pedersen_hash2(&b, &a, k);
  pedersen_hash2(out, &b, &zero);
}

/* The y word of the point with the x word X. Returns 1, or 0 when X is p or
   more or X^3 + 3 is not a square mod p. */
static int root(U256 *y, const U256 *x) {
  Fe fx, t, three, r;
  U256 e;
  G1 pt;
  if (!fe_from_u256(&fx, x, &BN254_FP)) return 0;
  fe_mul(&t, &fx, &fx, &BN254_FP);
  fe_mul(&t, &t, &fx, &BN254_FP);
  fe_from_u64(&three, 3);
  fe_add(&t, &t, &three, &BN254_FP);
  dec(&e, "5472060717959818805561601436314318772174077789324455915672259473661306552146");
  fe_pow(&r, &t, &e, &BN254_FP);
  fe_to_u256(y, &r);
  return g1_from_words(&pt, x, y);
}

static int key(int argc, char **argv) {
  G1 g, p;
  U256 x, px, py;
  if (argc != 3) return 2;
  dec(&x, argv[2]);
  pedersen_g(&g);
  g1_mul(&p, &g, &x);
  g1_to_words(&px, &py, &p);
  put(&px);
  put(&py);
  printf("\n");
  return 0;
}

static int point(int argc, char **argv) {
  U256 tag, poll, k, qx, qy;
  if (argc != 4) return 2;
  dec(&tag, argv[2]);
  dec(&poll, argv[3]);
  for (uint64_t i = 0; i < 1000u; i++) {
    u256_from_u64(&k, i);
    basex(&qx, &tag, &poll, &k);
    if (root(&qy, &qx)) {
      printf("%llu", (unsigned long long)i);
      put(&qx);
      put(&qy);
      printf("\n");
      return 0;
    }
  }
  return 3;
}

static int sign(int argc, char **argv) {
  G1 g, q, img, ring[KMAX], l, r;
  U256 tag, poll, k, qx, qy, ch, a, m, xs[KMAX], s[KMAX], c[KMAX], seed, au, h, ix, iy, one, kk, raw;
  Fe fa, fs, fc, fx, t1, sj;
  int bad = strcmp(argv[argc - 1], "bad") == 0;
  int n = argc - 8 - bad, j, at;
  if (n < 1 || n > KMAX) return 2;
  j = atoi(argv[7]);
  if (j < 0 || j >= n) return 2;
  dec(&tag, argv[2]);
  dec(&poll, argv[3]);
  dec(&k, argv[4]);
  dec(&qy, argv[5]);
  dec(&ch, argv[6]);
  basex(&qx, &tag, &poll, &k);
  if (!g1_from_words(&q, &qx, &qy)) return 3;
  pedersen_hash2(&a, &tag, &poll);
  pedersen_hash2(&m, &a, &ch);
  pedersen_g(&g);
  for (int i = 0; i < KMAX; i++) u256_from_u64(&s[i], 0);
  for (int i = 0; i < n; i++) {
    dec(&xs[i], argv[8 + i]);
    g1_mul(&ring[i], &g, &xs[i]);
  }
  g1_mul(&img, &q, &xs[j]);
  pedersen_hash2(&seed, &xs[j], &m);
  red(&fa, &seed);
  fe_to_u256(&au, &fa);
  g1_mul(&l, &g, &au);
  g1_mul(&r, &q, &au);
  h = m;
  chainpt(&h, &l);
  chainpt(&h, &r);
  at = (j + 1) % n;
  c[at] = h;
  for (int t = 1; t < n; t++) {
    U256 cn;
    u256_from_u64(&kk, (uint64_t)at + 100u);
    pedersen_hash2(&raw, &seed, &kk);
    red(&fs, &raw);
    fe_to_u256(&s[at], &fs);
    cn = c[at];
    step(&cn, &m, &g, &q, &img, &ring[at], &s[at]);
    at = (at + 1) % n;
    c[at] = cn;
  }
  red(&fc, &c[j]);
  red(&fx, &xs[j]);
  fe_mul(&t1, &fc, &fx, &BN254_FR);
  fe_sub(&sj, &fa, &t1, &BN254_FR);
  fe_to_u256(&s[j], &sj);
  if (bad) {
    u256_from_u64(&one, 1);
    u256_add(&s[0], &s[0], &one);
  }
  g1_to_words(&ix, &iy, &img);
  put(&ix);
  put(&iy);
  put(&c[0]);
  for (int i = 0; i < KMAX; i++) put(&s[i]);
  printf("\n");
  return 0;
}

int main(int argc, char **argv) {
  dec(&rmod, "21888242871839275222246405745257275088548364400416034343698204186575808495617");
  if (argc >= 2 && strcmp(argv[1], "key") == 0) return key(argc, argv);
  if (argc >= 2 && strcmp(argv[1], "point") == 0) return point(argc, argv);
  if (argc >= 9 && strcmp(argv[1], "sign") == 0) return sign(argc, argv);
  return 2;
}
