/* Pedersen commitments over BN254 G1 and the Sigma proofs that open them
   (docs/PLAN.md section 8, slice S9 Part 2). This is the host side:
   test/zktool.c prints the known-answer words, and the zk prelude (Part 3)
   checks the verifier equations in the guest language.

   Generators. G = (1, 2). H comes from try-and-increment (ruling O13):
   x = keccak256(tag || counter) mod p, tag = the 11 ASCII bytes
   "lend-lang/H" with no terminator, counter = a 32-byte big-endian word from
   0 in steps of 1, until x^3 + 3 is a square in Fp (0 counts as a square);
   y = the smaller of the two roots. Nobody knows log_G H.

   Words. A point is two U256 words (x, y); a scalar is one U256 word below
   r. Every byte string below uses 32-byte big-endian words, as the kernel
   hash op does.

   Hash chain. e = the kernel 2-word hash (keccak256 over 64 bytes) in a
   chain over the transcript words in order: h = hash(w0, w1), then
   h = hash(h, w) for each later word. The transcripts:
     Schnorr: G.x G.y H.x H.y C.x C.y A.x A.y
     bit:     C.x C.y A0.x A0.y A1.x A1.y
   The contract uses the raw chain output e as the ecMul scalar (0x07 reduces
   mod r). The prover reduces e mod r for the Fr arithmetic. The bit proof
   splits e mod r into e0 + e1, so the contract checks
   addmod(e0, e1, r) == addmod(e, 0, r) (Part 3, the op u256AddMod).

   Nonces. k = keccak256(secret || C.x || C.y || domain || index) mod r over 160
   bytes: secret = the 32-byte big-endian blind of C, then the two words of
   C, then the domain and index as 32-byte big-endian words. The domains
   prevent nonce reuse across proof protocols for the same opening:
     domain 1, Schnorr:  0 = k1, 1 = k2
     domain 2, bit:      0 = k, 1 = the simulated e, 2 = the simulated s
     domain 3, range:    i = the blind of bit i, i = 0..30
   No randomness. */
#ifndef LANG_FRONT_PEDERSEN_H
#define LANG_FRONT_PEDERSEN_H

#include <stddef.h>
#include <stdint.h>

#include "front/bn254.h"

#define PEDERSEN_BITS 32u
#define PEDERSEN_TAG "lend-lang/H"
#define PEDERSEN_TAG_LEN 11u

/* The generator G = (1, 2). */
void pedersen_g(G1 *out);
/* H and its counter. Returns 1, or 0 when no counter below 256 gives a
   square. */
int pedersen_h(G1 *out, uint64_t *counter);

/* The kernel hash of two words, byte for byte as the hash op. */
void pedersen_hash2(U256 *out, const U256 *a, const U256 *b);
/* The chain over COUNT words, COUNT 2 or more. */
void pedersen_chain(U256 *out, const U256 *words, size_t count);

/* C = v G + r H. Returns 1, or 0 when V or R is r or more. */
int pedersen_commit(G1 *out, const U256 *v, const U256 *r, const G1 *h);

/* A = k1 G + k2 H, s1 = k1 + e v, s2 = k2 + e r (mod r). The verifier checks
   s1 G + s2 H == A + e C with e raw. */
typedef struct {
  G1 a;
  U256 s1;
  U256 s2;
} SchnorrProof;

void pedersen_schnorr_challenge(U256 *e, const G1 *h, const G1 *c, const G1 *a);
/* Writes C and the proof. Returns 1, or 0 when V or R is r or more. */
int pedersen_schnorr_prove(SchnorrProof *out, G1 *c, const U256 *v, const U256 *r, const G1 *h);
int pedersen_schnorr_verify(const G1 *c, const SchnorrProof *proof, const G1 *h);

/* CDS OR proof that C = b G + r H with b in {0, 1}: branch 0 proves C = r H,
   branch 1 proves C - G = r H with -G = (1, p - 2). The verifier checks
   e0 + e1 == e (mod r), s0 H == A0 + e0 C and s1 H == A1 + e1 (C - G). */
typedef struct {
  G1 a0;
  G1 a1;
  U256 e0;
  U256 e1;
  U256 s0;
  U256 s1;
} BitProof;

void pedersen_bit_challenge(U256 *e, const G1 *c, const G1 *a0, const G1 *a1);
/* Writes C and the proof. Returns 1, or 0 when B is 2 or more or R is r or
   more. */
int pedersen_bit_prove(BitProof *out, G1 *c, uint64_t b, const U256 *r, const G1 *h);
int pedersen_bit_verify(const G1 *c, const BitProof *proof, const G1 *h);

/* Range proof for C = v G + r H with v below 2^32 (ruling O15): the Schnorr
   opening of C, one bit commitment C_i = b_i G + r_i H with a CDS OR proof
   per bit, and the sum of 2^i r_i equal to r (mod r). The verifier checks
   each proof and that the sum of 2^i C_i equals C. */
typedef struct {
  G1 c[PEDERSEN_BITS];
  BitProof bit[PEDERSEN_BITS];
  SchnorrProof open;
} RangeProof;

/* Writes C and the proof. Returns 1, or 0 when V is 2^32 or more or R is r
   or more. */
int pedersen_range_prove(RangeProof *out, G1 *c, const U256 *v, const U256 *r, const G1 *h);
int pedersen_range_verify(const G1 *c, const RangeProof *proof, const G1 *h);

#endif
