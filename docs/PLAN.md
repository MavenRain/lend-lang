# lend-lang plan

lend-lang is a Cubical TT language for a zero-knowledge lending contract
that a zero-knowledge self-referential DAO governs. A program declares one
system: a membership, a constitution, its aggregation and a private lending
ledger. The compiler checks the laws and writes the EVM bytecode of one
contract. The contract deploys to any EVM chain, for example Telcoin Network
or Ethereum mainnet. `design/DESIGN.md` holds the request and the design
verbatim.

## 1. Rulings

USER rulings (the request, 2026-10-17):

- Type formers: products, coproducts, monads (map/bind), algebras and
  coalgebras (fold/unfold), dependent types (Π, Σ, indexed families),
  quotients, paths, loops, HITs, QITs, HIITs, QIITs, as in Cubical TT.
- Base types and operations: only the ones in the pasted denotational
  design. We can add laws. We do not add base types or operations.
- Proof irrelevance is prohibited. lend-lang has no Prop-like types: no
  Prop, no SProp, no strict propositions, no irrelevant arguments or fields,
  no definitional proof irrelevance, no K, no UIP. A proposition is a type
  with an isProp proof, and that proof is data the kernel checks.
- Host: TinyCC.
- Target: EVM-deployable bytecode for any EVM chain.

USER rulings (S1, 2026-10-17):

- Every default in section 9 is RULED. Where an item has options, (a) is
  the ruling.
- The kit is hosts/tcc-evm-contract, and the kernel route is the
  orbit-lang lineage (O2). Tests are guest-language files plus `langc run`
  scripts. The kit gate scripts stay. We add no new harness.

## 2. The reference design

The design reuses the semantic core of MavenRain/self-referential-dao (local
clone `~/Documents/2026-06-uat-dao` at a965183, dependency
`~/Documents/unified-aggregation-theory`). orbit-lang docs/PLAN.md section 2
maps each reference name to its Lean definition with file:line cites. This
plan uses that map and does not copy it. The facts that matter here:

1. Over a discrete config category and a discrete target, a Lan along the
   orbit projection exists iff the rule is constant on orbits
   (U/Characterization.lean:81). In Cubical TT this is the premise of the
   quotient eliminator. The Lan unit is the computation rule.
2. `Gov L := orbitProjection act ⋙ L.functor` (SelfReferentialDAO.lean:146).
   `GovPhi (canonicalAmendment act) L = Gov L := rfl` (:186, :418).
3. The reference constitutions are constant, and the reference has no
   cryptographic layer and no ledger (orbit-lang PLAN section 2, facts 4, 5).

The two designs compared:

| Item | orbit-lang design | lend-lang design |
|---|---|---|
| Members | slot type Member, fixed | HIT Membership; path constructors admit and exit |
| Config | Member → Choice | the system Σ-type (M, F, L, sc, LS); voter configurations are Obj M |
| Constitution | Constitution | ChoiceRule (Obj M) D |
| Self-constitution | law 3, a guest theorem | IsSelfConstituting L, a field of Config; sc' is a PathP |
| Amendment | amend := aggregate ∘ gov | amendPath : Path Config c₀ c₁ |
| ZK | private ballots | private loan positions (Commitment, PrivateLoan, ZKProof) |
| Votes | castPrivate, proveZK, verifyZK, project | no vote operation; votes are the `proposal` of amendPath (O12) |

## 3. The design on the kit basis

The design gives a signature. Each name has one of three kinds (O4):

| Kind | Names | Meaning |
|---|---|---|
| slot | Agent, Choice, G, D, Position, Proposal fields | The program gives each one as a type built from formers, and gives the G-action on Choice. |
| defined | Membership, Obj, act, ChoiceRule, Aggregation, orbitProjection, Gov, IsFixedPointObj, IsSelfConstituting, LendingLedger, PrivateLoan, zkEquiv, Config, amendPath, borrow, repay, liquidate | The kernel declares Membership. The prelude defines the rest from the slots with formers. |
| primitive | Commitment (with its monoid), commit, ZKProof (with its check) | Opaque types and operations. The kit implements them over BN254 G1. |

Default readings (O5 to O11):

```text
2              := Unit + Unit
MemberSet      := Agent → 2                              -- Agent is a finite slot type
Membership     := HIT  base : MemberSet → Membership
                       admit a M : M a ≡ 0 → base M ≡ base (M[a := 1])
                       exit  a M : M a ≡ 1 → base M ≡ base (M[a := 0])
Obj M          := (Σ (a : Agent), M a ≡ 1) → Choice      -- a vote profile of the current members
act M          := the G-action on Obj M (on choices, and member permutation, O8)
Orbit M        := Obj M / act M                          -- QIT, set-truncated (orbit-lang S4)
orbitProjection x := [x]
ChoiceRule X D := X → D                                  -- a functor between discrete categories
Aggregation α F := Σ (L : Orbit M → D), (x : Obj M) → Path D (L [x]) (F x)
Gov L          := L.1 ∘ orbitProjection
IsFixedPointObj L := (X : Obj M) → Path D (Gov L X) (F X)
IsSelfConstituting L := IsFixedPointObj L                -- the ∃ L is the Σ over L in Config (O9)
Config         := Σ (M : Membership) Σ (F : ChoiceRule (Obj M) D)
                  Σ (L : Aggregation (act M) F) (IsSelfConstituting L × LendingLedger M)
```

On this reading the design laws become kernel facts or guest theorems:

| Law | Form in lend-lang |
|---|---|
| ⟦amend⟧ = Gov | the F field of c₁ is `Gov L` by definition; a refl path |
| anonymity | the aggregation factors through orbitProjection: `λ i. L.1 (glue g x i)` |
| self-constitution | `sc` is `L.2` read through the definition of Gov; `sc'` comes from transport-fill along amendPath, a PathP over IsFixedPointObj |
| lending homomorphism | the public face of each step is `c ↦ c ⊕ commit δ`, a monoid map (O13); the private face is related by zkEquiv (O14) |
| membership access | LendingLedger is a family over Membership, so a loan path needs a current member (O7) |

The contract never sees an Obj M value. It holds the orbit normal form (a
count per choice), the member set, one commitment per agent and the active
terms. Obj M, the private positions and every proof term are ghost (erased
at lowering).

## 4. The base span

Guest-visible base types: the slot, defined and primitive names of section
3, plus the Cubical TT primitives (the interval I, Path, PathP, the
universes, Glue). Guest-visible base operations: the design operations,
plus the Cubical TT primitives (path abstraction and application, transp,
hcomp, glue and unglue).

Kit items (default O3, the strict reading, as orbit-lang S3):

| Kit item | Plan |
|---|---|
| Nat, U256, Addr, Flag and their operations | Not guest-visible. Lowering carriers for counts, agent keys, epochs and proof words. |
| sealed-lang base ops natDiv, natMod, hash, ecrecover, ecAdd, ecMul, ecPairing, addrToU256 | Not guest-visible. The lowerer keeps them for the commitment group, the Fiat-Shamir hash and the proof check. |
| Map | Not guest-visible. The lowerer uses it for the commitment table. |
| `state`, Env, entries, views, pay, pull, events | Not guest-visible. The compiler writes the contract (section 7). |
| Eq formers | None. Path replaces them (sealed-lang R3, via the S2 port). |
| Former terms (pair, inl, fold, unfold, map, bind, and the rest) | Kept. These are formers, not base items. |
| sample domain Color, Item, Stack, Lot | None. The S2 port removes it. |

Amounts, rates and the collateral ratio are built from formers: Nat is the
initial algebra of `1 + X`, and the lowerer maps a Nat-shaped algebra to a
word (the zkpolicy-lang P7b-nat precedent).

## 5. The cubical kernel

lend-lang starts from the same kit as orbit-lang. No commit has touched
lang-template hosts/tcc-evm-contract since 0acacbd (2026-10-11). The
orbit-lang seed d7592fd is 2026-10-16. So the lend-lang seed src/ should be
byte-identical to the orbit-lang seed src/, and the orbit-lang kernel
commits should apply as cherry-picks (O2). S2 STEP 1 checks this with a
scratch probe.

| Slice | Source | Commits | Gives |
|---|---|---|---|
| S2 | orbit-lang | 7734007 | the sealed-lang S2 to S4 kernel: I, PathP, Path, transp, hcomp, HITs, path constructors, Trunc1, Trunc0, Quot, isProp, isSet; precompiles 0x01, 0x06, 0x07, 0x08 in the runner; Prop refusal rows |
| S3 | orbit-lang | 3a1ba76 (mechanism only) | the visibility tag, REFUSE_KIT, `langc --kit`; the orbit-lang signature rows are replaced by lend-lang rows |
| S4 | orbit-lang | 9e28399 | the orbit quotient as a kernel declaration after `act`; `elim` on Sum and Unit |
| S5 | orbit-lang S5 (coin-lang 44c917f, 91d7091, 34acff5) | a12973a | Fin, equivalences, Glue, ua, uaBeta, hcomp at Glue and U |
| S6 | orbit-lang S6, then new | 6fa6a15, 8dd82a3; new: O21 elim, Membership | HIITs and QIITs (orbit-lang: indexed families, blocks, block folds); dependent elim on a uniform HIT with PathP cases (O21); Membership as a kernel HIT |

orbit-lang reached S6 first (6fa6a15 and 8dd82a3: indexed families and
blocks), thus lend-lang S6 ports those commits. orbit-lang moved O21 to its
S7. lend-lang builds O21 and Membership in S6 (section 8, "Builder call
(S6)").

Proof irrelevance is prohibited (section 1, O20). The S2 port brings the
orbit-lang refusals: `Prop` and `SProp` are reserved names (NAME_UNKNOWN),
there is no irrelevance marker, two different Trunc1 or Trunc0 elements are
not convertible (PATH_END), and a path between paths needs a squash path,
never refl (PATH_UIP). Each later slice adds a refusal row for each
Prop-like form it could admit.

### Kernel rules carried from the lineage

The R-numbers in code comments come from the kit, coin-lang and sealed-lang.
orbit-lang PLAN section 5 "Kernel rules carried from sealed-lang" gives the
key. Do not renumber these cites. New lend-lang rulings use O-numbers.

## 6. The zero-knowledge lending ledger

### Membership and the ledger family

LendingLedger is a family over the HIT Membership. The eliminator into U
sends each path constructor to a path in U, so admit and exit must act on
the ledger by equivalences (ua). Adding a position on admit and deleting it
on exit is not an equivalence. Default reading (O7):

```text
State    := Active Position + Forfeited Position + Empty
Ledger M := Σ (s : Agent → State), (a : Agent) → Inv (M a) (s a)
            -- Inv 1 s: s is Active or Empty; Inv 0 s: s is Forfeited or Empty
admit a : swap Forfeited p ↦ Active p at a      (an equivalence)
exit  a : swap Active p ↦ Forfeited p at a      (its inverse)
```

So transport along an exit forfeits the position of the exiting member.
This is the design's "an exit ... invalidates subsequent private loans for
the exiting member by transport". A readmitted member gets the forfeited
position back. This needs Glue and ua (S5) and the dependent eliminator on
a HIT (S6, O21).

### Commitments and the Glue face

Default commitment (O13): Pedersen over BN254 G1,
`commit (p, r) := p·G + r·H`. Commitment is the group (G1, +, 0), so commit
is a monoid homomorphism, as the design's last law asks. Hiding and binding
rest on discrete log. That is an assumption in docs/VALIDATION.md, not a
guest theorem.

The design's `PrivateLoan c = Glue c (φ = i1 ↦ (P , zkEquiv))` with
`zkEquiv : P ≃ c` is ill-typed and false as written: c is a term, not a
type, and a hiding commitment is not injective on positions (the blind is
lost). Default reading (O14), the first isomorphism theorem:

```text
Opening      := Position × Blind
PrivateFace  := Opening / (o ~ o' iff commit o ≡ commit o')      -- QIT
PublicFace   := Σ (c : Commitment), ‖fibre commit c‖₁           -- the image
zkEquiv      : PrivateFace ≃ PublicFace                          -- a guest theorem
Loan i       := Glue PublicFace [i = i1 ↦ (PrivateFace, zkEquiv)]
PrivateLoan c := Σ (x : Loan i1), Path Commitment (unglue x).1 c
```

S10 Part 1 (2026-10-20): the guest spelling in examples/zk.lang is
`Loan : Path (Type 0) PublicFace PrivateFace := fun (i : I) => zkLoanPath
@ ~i` (a definition typed `I -> Type 0` is refused by rule R9, so `Loan i`
is `Loan @ i`), and `(unglue x).1` is `commit' x`: `Loan @ i1` reduces to
PrivateFace by the true-key rule, so `unglue` at that type is UNGLUE_TYPE;
at the true key `unglue` is the branch equivalence `forward`, whose first
component is `commit' x`. The guest name is `zkPrivateLoan`: `PrivateLoan`
stays a kernel-given signature name (REFUSE_SIG), as `zkEquiv` became
`zkFaceEquiv`.

S10 Part 2 (2026-10-20): the ledger-level reading of the borrow, repay
and liquidate rows of the table below. δ is the position delta.
`zkBorrowL m δ` and `zkRepayL m δ` step the S7 kernel Ledger at the
member: the holding `Active p` becomes `Active (u256Add p δ)` or
`Active (u256Sub p δ)`, every other holding keeps its value, and the Inv
field moves by the elim on the holding (`stepOne`). `u256Sub` saturates,
so a repay above the position lands at `Active 0u`: that is the "debt
does not go below 0" law of the repay row (the eval row
`zkRepayUnderKA`). These are pure transformations at the fixed agent
`inr unit`. The S9 EVM entries `zkBorrow` and `zkRepay` verify an opening
of a submitted replacement commitment; they do not check its relation
to the previous commitment or a position delta. Connecting these ledger
transformations to the contract, including the commitment update and
the treatment of excess repayment, remains pending (Part 4: below). Saturation alone
does not establish `c' = c ⊖ commit (δ, r)` for δ above the debt.
`zkLiquidateL` is the transport along `exitOne`, the exit
path of the member in LendingLedger (`Ledger onlyOne -> Ledger nobody`):
a member cannot hold Forfeited, so the ledger-level liquidate leaves the
member set and the former member holds `Forfeited 7u`. The design names
`borrow`, `repay` and `liquidate` stay kernel-given signature names
(REFUSE_SIG), as `PrivateLoan` did, so the guest terms carry the suffix
L: `zkBorrowL`, `zkRepayL`, `zkLiquidateL`.

S10 Part 3 (2026-10-20): the host and EVM differential of the zk
entries, the S10 gas table and the EIP size rows. test/run/zk.script has
8 calls: enroll, borrow, repay, liquidate at 9 after the repay, borrow
again, liquidate at 8 and at 9, then borrow on the Forfeited position.
`langc run` gives ok, ok, ok, revert, ok, revert, ok, revert
(test/run/zk.out), and test/diff.sh gets the same result at each step and
the same end storage on the chain. The chain of record is the local
go-ethereum `evm` 1.14.12 with the test config of test/evmchain.sh: each
fork at block 0, PUSH0 on, the genesis gas limit 30,000,000. A
public-chain probe (the code at 0x06 and 0x07, the EIP-170 limit of that
chain) needs the network, so it is a USER step or a later part.

| Item | Bytes | Limit | Used |
|---|---|---|---|
| initcode (`langc --kit build examples/zk.lang`) | 7532 | EIP-3860 49152 | 15.3 % |
| runtime (`--runtime`, the deployed code) | 7512 | EIP-170 24576 | 30.6 % |

The constructor is the 20-byte difference (10 ops, 1,548 gas).
test/build.sh bounds both sizes by their limits.

The gas is the `EVM gas used` line of `evm run --statdump`: execution gas
only, with no 21,000 intrinsic gas and no calldata gas. The step numbers
count the calls of test/zk.sh (the gas rows are not counted). The five
rows marked "row" are exact gas rows in test/zk.sh (`gasRow`).

| Step | Call | Result | Gas |
|---|---|---|---|
| deploy | create, 7512 bytes deposited | ok | 1,503,948 (derived) |
| 8 | zkEnroll, first | ok | 44,558 |
| 9 | zkBorrow, wrong opening | revert | 24,557 |
| 10 | zkBorrow, non-member | revert | 2,553 |
| 11 | zkDepart | ok | 10,379 |
| 12 | zkBorrow, Forfeited | revert | 2,553 |
| 13 | zkEnroll, again | ok | 27,458 |
| 14 | zkBorrow, point not on the curve | revert | 29,531,553 |
| 15 | zkBorrow, first (row) | ok | 100,353 |
| 16 | zkRangeBit overflow | revert | 7,793 |
| 17 | zkRangeDone early | revert | 6,938 |
| 18 | zkRangeBit rb31, first | ok | 112,242 |
| 19-49 | zkRangeBit rb30 to rb00 (row on rb00) | ok | 60,942 each |
| 50 | zkRangeDone (row) | ok | 46,800 |
| 51 | zkRepay (row) | ok | 46,247 |
| 52 | zkLiquidate at 9, no deadline | revert | 6,923 |
| 53 | zkBorrow, second | ok | 60,553 |
| 54 | zkLiquidate at 8 | revert | 7,135 |
| 55 | zkLiquidate at 9 (row) | ok | 10,113 |
| 56 | zkBorrow, Forfeited | revert | 4,732 |

The 32-bit range chain is 112,242 + 31 x 60,942 + 46,800 = 2,048,244
gas, so a first private borrow with its range proof is 2,148,597
execution gas over 34 transactions. The deploy gas is derived: on the
create path, `evm run --statdump` prints the gas limit minus the gas used
(28,496,052 at 30,000,000), so the deploy gas is 1,503,948 = 7512 x 200
(the code deposit) + 1,548 (the constructor). Step 14 is a known cost,
with no row and no lowering change in Part 3. The 0x07 STATICCALL
forwards all gas (src/evm.c), and a failing precompile uses all the gas
that it gets. Thus a caller who submits a point that is not on the curve
pays 29,531,553 = 63/64 x 30,000,000 + 303. A cap on the gas to 0x06 and
0x07 is a lowering change for a later part.

S10 Part 4 (2026-10-20): the ledger/contract integration. zkBorrow and
zkRepay take the public delta d and a blinding r (two U256 words) and
compute the new commitment on the chain. `zkShift s a d r` is
c(a) ⊕ commitWords (d, r): the two ecMul of commitWords and one ecAdd.
zkBorrow stores c ⊕ commit (d, r), the `zkBorrowL` step. zkRepay stores
c ⊕ commit (q - (d mod q), q - (r mod q)) = c ⊖ commit (d, r), the
`zkRepayL` step, with q = `zkOrder`, the BN254 group order.
`zkNegScalar` reduces each U256 word with `u256AddMod` before subtraction:
`u256Sub q x` alone saturates at zero for x > q and cannot negate that
scalar. At d = r = 0 the shift point is
q G + q H, the identity (0, 0), so no zero case is necessary. The
Schnorr opening proof leaves both entries. The caller submits no point,
so the wrong-opening row and the off-curve row of Part 3 (steps 9 and
14) leave test/zk.sh. zkRangeBit and zkSchnorrView still take points, so
the gas cap on 0x06 and 0x07 stays an item for a later part.

The cost is privacy. δ is public, so a watcher who adds the deltas
knows the position: a loan that uses these entries loses the S9 hiding
of the position value. Only a hidden δ with a range chain for each δ
keeps it (a later design). The excess law: a repay above the debt wraps
mod q on the chain, where the ledger saturates at `Active 0u`
(`zkRepayUnderKA`). The payer loses the excess e, the stored point is a
commitment to q - e, and no 32-bit range chain opens it, so the position
stays unranged. The link rows of test/eval/expect.txt connect the two
levels on the review state that holds cKA = commit (7, 11):
`zkBorrowLink` (borrow 5u 13u gives commit (12, 24), and 12u is
`zkBorrowKA`), `zkRepayLink` (repay 5u 5u gives commit (2, 6), and 2u
is `zkRepayKA`), `zkRepayExcessLink` 0 and `zkRepayExcessWrap` 1 (repay
9u 5u gives commit (q - 2, 6), not the commit (0, 6) of the saturated
ledger). Repayment clears the deadline only when both commitment coordinates
become zero, the identity. Zero, partial and excess repayments with a nonzero
result keep the existing deadline and remain liquidatable after it. A
full cancellation must also cancel the accumulated blinding; a residual
blinding keeps the deadline. The membership map: `zkPosActive` (a member with status 1) is
the ledger member set. zkLiquidate sets status 2 and keeps the member
flag, and `zkLiquidateLink` (0) shows that the liquidated address leaves
that set, as `zkLiquidateL` goes from `onlyOne` to `nobody`. The zkEnroll
guard closes the readmit path: zkEnroll reverts when status a = 2 and
deadline a >= 1 (a liquidated position). The prelude has no negation, so
the guard is `flagIf (zkAnd (natEq (mapGet (status s) a) 2) (natLe 1
(mapGet (deadline s) a))) none (some ...)`. zkDepart preserves the deadline;
enroll after depart stays ok only when that deadline was already 0.
`zkEnrollForfeited` (0) and the
third-party zkEnroll row of test/zk.sh show the revert.

test/run/zk.script has 25 calls: the 8 calls of Part 3 on the delta
arguments 7u 11u, then bb enrolls the Forfeited aa (revert), and bb
enrolls, borrows 5u 13u and repays 20u 13u (ok, the wrap). The next
13 calls check liquidation after excess, zero and partial repayment,
reenrollment refusals, and a borrow/repay round trip with both scalars q + 1
that returns to the identity and clears its deadline. test/zk.sh
adds six rows from bb: enroll, borrow 5u 13u, borrow 7u 13u, repay 5u
13u, the excess repay 20u 13u and the third-party zkEnroll of aa
(revert), then the same 13 regression calls. Ten further eval rows check
the repayment deadlines, partial liquidation, independent d and r above q,
and round trips at q - 1, q and the maximum U256 word.
The zk chain has 78 steps (73 calls and 5 gas rows). The S10
gas table after Part 4 (the same rules as the Part 3 table):

| Item | Bytes | Limit | Used |
|---|---|---|---|
| initcode (`langc --kit build examples/zk.lang`) | 7522 | EIP-3860 49152 | 15.3 % |
| runtime (`--runtime`, the deployed code) | 7502 | EIP-170 24576 | 30.5 % |

| Step | Call | Result | Gas |
|---|---|---|---|
| deploy | create, 7502 bytes deposited | ok | 1,501,948 (derived) |
| 8 | zkEnroll, first | ok | 44,684 |
| 9 | zkBorrow, non-member | revert | 2,493 |
| 10 | zkDepart | ok | 10,379 |
| 11 | zkBorrow, Forfeited | revert | 2,493 |
| 12 | zkEnroll, again | ok | 29,769 |
| 13 | zkBorrow 7u 11u, first (row) | ok | 107,220 |
| 14 | zkRangeBit overflow | revert | 7,793 |
| 15 | zkRangeDone early | revert | 6,938 |
| 16 | zkRangeBit rb31, first | ok | 112,242 |
| 17-47 | zkRangeBit rb30 to rb00 (row on rb00) | ok | 60,942 each |
| 48 | zkRangeDone (row) | ok | 46,800 |
| 49 | zkRepay 7u 11u, full (row) | ok | 45,467 |
| 50 | zkLiquidate at 9, no deadline | revert | 6,923 |
| 51 | zkBorrow 7u 11u, second | ok | 107,220 |
| 52 | zkLiquidate at 8 | revert | 7,135 |
| 53 | zkLiquidate at 9 (row) | ok | 10,113 |
| 54 | zkBorrow, Forfeited | revert | 4,672 |
| 55 | zkEnroll bb | ok | 44,684 |
| 56 | zkBorrow 5u 13u, bb first | ok | 107,220 |
| 57 | zkBorrow 7u 13u, bb second | ok | 53,120 |
| 58 | zkRepay 5u 13u, partial | ok | 39,892 |
| 59 | zkRepay 20u 13u, excess (wraps) | ok | 39,892 |
| 60 | zkEnroll aa by bb, Forfeited | revert | 4,595 |

The table covers the original 60 calls; the 13 regression calls follow them.
The full repay at step 49 returns the commitment to the identity, so it
clears the cx and cy slots, and the second borrow (step 51) pays the
same slot writes as the first. A first borrow with its range
proof is now 107,220 + 2,048,244 = 2,155,464 execution gas over 34
transactions. The deploy gas is 7502 x 200 + 1,548 = 1,501,948.

zkEquiv needs no cryptographic assumption, so transport along `Loan`
computes. The holder keeps a representative opening. The contract sees
only the public face.

### Proofs and steps

Default proof system (O15): Sigma protocols over the Pedersen group, with a
keccak256 Fiat-Shamir challenge. A Schnorr proof shows knowledge of an
opening. A range check uses a bit decomposition with one OR proof per bit.
The verifier needs only ecAdd (0x06), ecMul (0x07) and, for the per-bit
challenge split, the ADDMOD opcode (S9 ruling, O15). There is no trusted
setup and no circuit compiler, and the TinyCC host runs the prover. Cost:
about 4 ecMul per range bit; the S10 gas table gives the numbers.

| Step | Public face (contract) | ZKProof shows |
|---|---|---|
| borrow δ | `c' = c ⊕ commit (δ, r)`; the agent is a member and Active | opening of c'; collateral · den ≥ debt · num, for the active ratio num/den |
| repay δ | `c' = c ⊖ commit (δ, r)` | opening of c'; debt does not go below 0 |
| liquidate | the position is Forfeited after the grace window (O17) | none (the holder failed to post a health proof) |

The collateral ratio and the rate come from D, the outcome of the
aggregation (O10). So the DAO governs the lending terms, and an amendment
can change them. Each step is a path in LendingLedger M whose ends satisfy
the invariants; the contract checks the proof in place of the private ends.

## 7. The contract

The compiler writes one contract from the system declaration (default O18,
O19).

| Entry | Effect |
|---|---|
| constructor (agent keys, member set, initial terms) | stores the agent keys (G1 points), the member set, the terms, poll id 0 |
| `vote(choice, signature)` | adds one coordinate of the proposal configuration to the count per choice (O12) |
| `amend(admits, exits)` | amendPath: applies the membership path, installs `Gov L` and the terms it selects, transports the ledger (exits forfeit), opens a grace window, increments the poll id, sets the counts to zero |
| `borrow(c', proof, signature)`, `repay(c', proof, signature)` | the steps of section 6; a bad proof or a non-member reverts |
| `liquidate(agent)` | after the grace window, a position without a fresh health proof becomes Forfeited |
| views | `terms()`, `commitmentOf(agent)`, `isMember(agent)`, `selfConstituting()` |

The contract holds no tokens by default (O18): the design admits no deposit
or withdraw operation, so the ledger is a private credit ledger in one
abstract unit. A constitution is code, so the contract holds a finite menu
of compiled constitutions, and amend selects an index (the orbit-lang O6
defunctionalization). Each menu entry carries its invariance proof at
compile time; the proof is erased in the bytecode.

## 8. Slices and gates

Each slice ends with `make check` GREEN, mutants one at a time (`make -B`
for each build), the counts in docs/STATUS.md, and a USER commit.

| Slice | Content | Gate additions |
|---|---|---|
| S0 | seed: design, plan, kit scaffold | kit gate GREEN |
| S1 | rulings on O1 to O26: DONE 2026-10-17, all defaults | none |
| S2 | port orbit-lang 7734007 (kernel, precompiles, sample domain removed): DONE 2026-10-17 | the orbit-lang S2 Prop-like refusal rows; precompile vectors |
| S3 | base-span pruning (mechanism of orbit-lang 3a1ba76); the lend-lang signature declared: DONE 2026-10-17 | REFUSE_KIT rows for each kit name and sealed-lang base op; REFUSE_SIG rows for the lend-lang signature |
| S4 | port orbit-lang 9e28399: Orbit after actChoice (choices only; the M index waits for Obj and act), elim on Sum and Unit: DONE 2026-10-18 | Orbit fold without isSet, wrong boundary, Orbit path at run time |
| S5 | Glue, ua, uaBeta, hcomp at Glue and U (port of orbit-lang a12973a): DONE 2026-10-18 | Glue boundary mismatch, ua on a non-equivalence |
| S6 | HIITs, QIITs, dependent elim on a HIT with PathP cases; Membership as a kernel HIT: DONE 2026-10-18 | wrong index in a path constructor, inductive-inductive order, a missing PathP case, admit without `M a ≡ 0` |
| S7 | DAO prelude: Obj, act, ChoiceRule, Aggregation, Gov, IsFixedPointObj, Config, amendPath; LendingLedger over Membership by ua: DONE 2026-10-18, without amendPath (O27) | a non-invariant constitution, a wrong unit, an exit that does not forfeit, the ∃ of IsSelfConstituting as a Prop |
| S8 | port orbit-lang B1 676c7a8 (dependent elim for domain families) and B2 eafa1df (outcomeSet and menu slots, lend names, separate splice); O27 option (b) probe, then amendPath if it clears; law 2 uniqueness (USER ruling S8): DONE 2026-10-18, the guest term is amendExit (amendPath stays reserved) | the orbit-lang B1 rows (elim-*); the B2 rows with lend names (outcome-set-type, menu-type, dao-no-set, dao-redefine); amendPath rows if O27 (b) clears |
| S9 | ZK ledger: Pedersen on BN254 in C, Sigma-protocol verifier lowering, zkEquiv, PrivateLoan, borrow, repay, liquidate (moved from S8, USER ruling S8) | known-answer vectors; mutants: wrong opening, range overflow, non-member step, step on a Forfeited position, point not on the curve |
| S10 | contract end to end: deploy, vote, amend, borrow, repay, liquidate; host and EVM differential; gas table; chain probe | differential rows, revert rows, EIP-170 and EIP-3860 size limits; Part 1 Loan and zkPrivateLoan DONE 2026-10-20; Part 2 pure ledger borrow, repay, liquidate DONE 2026-10-20; Part 3 differential, gas table, EIP rows DONE 2026-10-20; Part 4 ledger/contract integration DONE 2026-10-20 |
| S11 | regimes (optional, O23), docs, final gate | regime rows |

Builder call (S2, 2026-10-17): the port of orbit-lang 7734007. Identity
check: on d4fdb2e, the trees src, test, domain, examples, Makefile,
formers, gen and probe are equal to the trees of orbit-lang d7592fd and
6cd1104 (`git rev-parse <rev>:<path>` in both repos). The patch is
`git -C ~/Documents/orbit-lang diff 6cd1104 7734007 -- .` without
docs/PLAN.md, README.md and design (103 files, +4306 -425), applied with
`git apply --3way`. A scratch clone probe and the repo apply gave 0
conflict hunks. After the apply, the same eight trees plus
docs/VALIDATION.md are equal to the trees of orbit-lang 7734007.
docs/host/README.md changes in two lines that named orbit-lang as this
repo. The gate `make -B check` is GREEN, and each count is equal to the
orbit-lang S2 count (docs/STATUS.md section S2). S2 does not prune; S3
prunes ("port first, prune second").

Builder call (S3, 2026-10-17): the mechanism of orbit-lang 3a1ba76 and the
lend-lang signature. Identity check: on 2449b67 (the USER S2 commit), the
trees domain, examples, Makefile, formers, gen and probe are equal to the
trees of orbit-lang 7734007. The trees src and test and the file
docs/VALIDATION.md are different only by the USER review fix of 2449b67:
`conv_system`, `conv_branches` and `mark_intervals` in src/front/eval.c,
4 roundtrip rows and 1 refusal row (`rejected`) in test/pathtool.c, and a
2026-10-17 paragraph in docs/VALIDATION.md. orbit-lang has no
`conv_system` in a commit. USER ruling (S3 session 1, 2026-10-17):
continue, and keep the fix as a known lend-lang-only delta. The patch is
`git -C ~/Documents/orbit-lang diff 7734007 3a1ba76 -- .` without
docs/PLAN.md, README.md, design and docs/STATUS.md (58 files, +597 -161),
applied with `git apply --3way`. A scratch clone probe and the repo apply
gave 1 conflict file, docs/VALIDATION.md, with 2 prose hunks (the "Host
executable" line, and the review-fix paragraph against the orbit-lang S3
paragraph). Both hunks are resolved by hand: both paragraphs stay, and the
executable line is "at 2449b67 plus the S3 changes". There are 0 code
conflicts. After the apply, the trees src, test, domain, examples,
Makefile, formers, gen and probe are equal to the trees of orbit-lang 3a1ba76,
except src/front/eval.c and test/pathtool.c (the review fix). The orbit-lang
README.md does not change from 7734007 to 3a1ba76; the `--kit` usage lines
are in docs/host/README.md, which the patch applies. The signature swap
replaces the orbit-lang SIGNATURE table of src/front/check.c with the
lend-lang table (18 types, 13 operations; docs/STATUS.md section S3) and
the 6 orbit-lang REFUSE_SIG rows with 16 lend-lang rows, and adds 4 Prop
rows. The wf-builder stage (Fable xhigh) died on a 429 usage limit
(req_011Cg8rYD9gsEoBrNUVtitST), and the opus xhigh fallback died on the
`reasoning_extraction` classifier (req_011Cg8rcsqDwcS29CFnEXgZc), both
before a file change. Thus the swap is built by hand, as orbit-lang S3 to
S5 did. formers/tcc-evm-contract.md line 7 names lend-lang S3. The gate
`make -B check` is GREEN, exit 0: examples 25, parse refusals 19, check
refusals 133, kit-only refusals 22, evals 62, and the other counts equal
to S2. The mutant loop kills 20 of 20 mutants (docs/STATUS.md section S3).

Builder call (S4, 2026-10-17): the port of orbit-lang 9e28399. On ee9b215
(the USER S3 commit), the patch is `git -C ~/Documents/orbit-lang diff
3a1ba76 9e28399 -- .` without docs/PLAN.md, README.md, design and
docs/STATUS.md (27 files, 41 hunks, +536 -18), applied with `git apply
--3way` after a fetch of the orbit-lang objects. The apply gave 3
conflict files, each resolved by hand: SPEC.md (1 hunk: the orbit-lang
F16 text with the lend-lang name), test/check/expect.txt (1 hunk: the
lend-lang S3 rows and the 13 orbit-lang S4 rows stay, the orbit-lang
`sig-act-*` rows do not) and docs/VALIDATION.md (2 hunks: the counts go
to S4, the lend-lang S3 and review paragraphs stay, and an S4 paragraph
is added). There are 0 code conflicts, and the eval.c `conv_system` fix
of 2449b67 composes with the patch. Adaptation by hand: (1) in
src/front/front.c the kernel puts Orbit and orbitProj after the program
`def actChoice` (orbit-lang: `def act`), function
`orbit_after_act_choice`, and ORBIT_R is the relation of Quot over Agent
-> Choice: R c d = Sigma (g : G) (Path (Agent -> Choice) (fun (a :
Agent) => actChoice g (d a)) c). (2) In src/front/check.c the kernel_sig
comment and message say "after actChoice". The review before 993ee13 adds
`orbitProj` as SIG_OP/SIG_DEFINED, reserving it for program events, state
constructors and projections even before actChoice. The other SIGNATURE
rows do not change: `act`, `Orbit` and `orbitProjection` stay SIG_DEFINED. `act`
stays reserved, thus the examples use a helper `actObj` (actChoice at
each agent). (3) examples/orbit.lang, test/lower/orbit-word.lang,
test/check/orbit-*.lang and test/check/orbitproj-guest-def.lang use the
lend-lang names: Member is Agent, Group is G, Outcome is D, and `def act`
is `def actChoice` plus `def actObj`. Design note: section 3 gives Orbit
M := Obj M / act M, over the current members, with member permutation
(O8). S4 ports the orbit-lang form: Orbit over all of Agent -> Choice,
choices only. The M index and the member permutation wait for the slice
that defines Obj and act (S7). In session 1 the wf-builder stage (Fable
xhigh) died on a 429 usage limit (req_011Cg9C2NqbCP9jH4Baoyh7o) and the
opus xhigh fallback died on the `reasoning_extraction` classifier
(req_011Cg9C5T7u3seuat8mYPr2E), both before a file change. Thus the port
is built by hand. In session 2 the records builder (Fable xhigh) died on
a 429 (req_011Cg9HyC5ZJvfdiGSiWPMCQ) and its opus xhigh fallback died on
`reasoning_extraction` (req_011Cg9JH3RfDR42Tgz8EYv17), both before a file
change, thus the mutants and the records are by hand too. Session 2 adds
one kit row to test/fronttool.c, `state State := Orbit` (REFUSE_SIG).
sig_member_free reads the SIGNATURE rows, and kernel_sig refuses `def
Orbit` before the table, thus `sig-def-orbit` alone does not kill a
removed Orbit row (mutant K14r). The USER committed the code as 993ee13;
it also includes three orbitProj kit refusals and two positive fronttool
cases for event-local fields and trusted domain declarations. The original
mutant loop predates the orbitProj SIGNATURE row; STATUS section S4 records
that snapshot separately. The gate `make -B check` is GREEN, exit 0:
examples 26, parse refusals 19, check refusals 146, kit-only refusals 22,
evals 65, lower refusals 15, build checks 32, and the other counts equal
to S3. The mutant loop kills 15 of 17 mutants (docs/STATUS.md section
S4).

Builder call (S5, 2026-10-18): the port of orbit-lang a12973a. On 68ce871
(the USER S4 records commit on 993ee13), the patch is `git -C
~/Documents/orbit-lang diff 9e28399 a12973a -- .` without docs/PLAN.md,
README.md, design and docs/STATUS.md (38 files, 83 hunks, +1527 -76),
applied with `git apply --3way` (scratch probe first, same result). The
apply gave 5 conflict files, each resolved by hand: src/front/eval.c (2
hunks: the lend-lang conv_system, conv_branches and mark_intervals of
2449b67 stay, the orbit-lang S5 conv_system goes out, and conv_loop
keeps one call because the lend-lang rule is symmetric), test/pathtool.c
(3 hunks: both sides stay), SPEC.md (1 hunk: the lend-lang name with
F17), examples/orbit.lang (1 hunk: the lend-lang `agreeFlipped` with
actObj stays and the orbit-lang S5 lines are added) and
docs/VALIDATION.md (2 hunks). No added line uses act, Member, Group or
Outcome, thus the new files need no name change. The wf-builder stage
(Fable xhigh) died on a 429 (req_011Cg9QZ7NBC7zTpR19EajAp) and the opus
xhigh fallback died on `reasoning_extraction`
(req_011Cg9Qc1RLKE6USFQBBTFi5), both before a file change. Thus the
port, the mutants and the records are by hand. The gate `make -B check`
is GREEN, exit 0: examples 29, check refusals 161, evals 75, lower
refusals 16, build checks 33, and the other counts equal to S4. The
mutant loop kills 10 of 11 mutants (docs/STATUS.md section S5).

Builder call (S6, 2026-10-18). Three parts. Kernel survey:
~/Documents/lend-lang-work/S6-survey-kernel.md (orbit-lang 8dd82a3 cites).
Facts: `elim` is a builtin `elim P c1 .. cn x` for Sum and Unit only
(rule_elim, one TYPE_MISMATCH for other scrutinees); fold has no motive
and checks a path case as `PathP (fun _ => RES) B0 B1`; a fold boundary
end must be a field, a path field at an interval, or an earlier point
constructor (HIT_BOUNDARY otherwise); there is no decidable equality and no
function update; Fin is VIS_INTERNAL; the S4 Orbit splice parses fixed
source text with origin ORIGIN_DOMAIN after the program def.

Part 1, the port (session 1). On 4484ce6 (the USER S5 commit on 68ce871),
the patch is `git -C ~/Documents/orbit-lang diff a12973a 8dd82a3 -- .`
without docs/PLAN.md, README.md, design and docs/STATUS.md (14 files, 64
hunks, +635 -76), applied with `git apply --3way` (scratch probe first,
same result). The apply gave 1 conflict file, test/fronttool.c (1 hunk:
the lend-lang `kit` helper and the orbit-lang `normal_roundtrip_in` both
stay). There are 0 code conflicts in src. The port gives the orbit-lang S6
design (orbit-lang PLAN section 8, "Builder call (S6, 2026-10-17)"):
index telescopes, constructor result annotations, `and` blocks, Size,
Bag, Charter and Ratified in domain.lang, block folds, examples/hiit.lang
and qiit.lang, the refusal codes HIT_INDEX ("a wrong index in a path
constructor") and BLOCK_ORDER ("inductive-inductive order"). The orbit-lang
S6 rows that are not in a commit (its B4 rows) are not ported. The gate
`make -B check` is GREEN, exit 0: examples 31, check refusals 162, evals
80, and the other counts equal to S5.

Part 2, O21 (new). `elim` gets a third scrutinee class: a uniform domain
family whose constructors are points and 1-dimensional path constructors.
- Surface: no parser change. `elim P c1 .. cn x`, one case for each
  constructor in declaration order. The motive is `P : F params -> Type k`.
- Point case of `c (x1 : A1) .. (xk : Ak)`: a function of the fields in
  order, and each recursive field (type `F params`) is followed directly
  by its hypothesis `P x`. It returns `P (c x1 .. xk)`.
- Path case of `p (fields) (i : I) [ ~i -> e0, i -> e1 ]`: a function of
  the fields and hypotheses as above (no interval argument). It returns
  `PathP (fun i => P (p fields i)) E0 E1`. Ek is the hypothesis of ek
  when ek is a recursive field, and the point case at the arguments of ek
  (with their hypotheses) when ek is a point constructor application.
- Refusals: a missing or extra case is TYPE_MISMATCH at the elim case
  count (gate row "a missing PathP case"); a path case with wrong ends
  fails the check against its PathP type. A family with a 2-dimensional
  constructor (a squash), an indexed or block family, a constructor with a
  path field over the family, Quot and Orbit stay TYPE_MISMATCH at the
  one rule_elim site (USER ruling S6). No new refusal code.
- Eval (reduce_elim): a point constructor value applies its case to the
  fields and the hypotheses (each hypothesis is elim at the recursive
  field). A path constructor value at an interval value r applies its
  case, then `path_apply` at r with the ends E0 and E1, as fold_path does.
  A neutral or a formal hcomp scrutinee stays neutral (a dependent hcomp
  over the motive needs comp; not in S6).
- Lowering: no change. elim at a word position is REFUSE_LOWER.
- Fix (session 3): in check.c `elim_family_case`, the empty Env is NULL.
  For a family with no parameters (Membership, S1), the field loop
  (the `env != NULL` test) stopped at the first field, and the function
  returned NULL with no diagnostic (`langc: (null): :`). Now the flag
  `ok` records a failed env_push, which gives the OOM diagnostic.

Part 3, Membership (new). src/front/front.c splices fixed source text with
origin ORIGIN_DOMAIN directly after the program `def Agent` (the
orbit_after_act_choice precedent; USER ruling S6). With 2 := Sum Unit
Unit, 0 := inl unit and 1 := inr unit, the text declares:

```text
family Membership :=
    base (m : Agent -> Sum Unit Unit)
  | admit (m n : Agent -> Sum Unit Unit) (a : Agent)
          (p : Path (Sum Unit Unit) (m a) (inl unit))
          (q : Path (Sum Unit Unit) (n a) (inr unit))
          (r : (b : Agent) -> Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b)))
          (i : I) [ ~i -> base m, i -> base n ]
  | exit  (as admit, with inl and inr swapped in p and q)
```

The premises p, q and r say that n is `M[a := 1]` (exit: `M[a := 0]`) up
to paths, thus no decidable equality on Agent is needed and both ends are
point constructors at fields (USER ruling S6). There are no 2-cells (O6
(a)). Each field name has the constructor name as prefix (admitM,
admitP and the rest), thus no field name is a global name (the
orbit-lang domain.lang precedent). A program without `def Agent` has no
Membership.

Deviation 1 (session 2): the point constructor is `memberSet`, not
`base`. S1 owns `base` (REFUSE_NAME, rule R4).

Deviation 2 (session 3): there is no elim row into a 2-valued type.
admit and exit join each member set to each other member set, thus a
consistent map from Membership to Sum Unit Unit is constant and its path
cases are refl. A refl case cannot show a difference between path_apply
at r and path_apply at i0. Thus the eval rows use copy (into
Membership) and wind (into S1).

Rows (session 3): examples/membership.lang (Agent, nobody, onlyOne,
sameElse, sameElseBack, admitOne, exitOne, loopAt (a dependent elim
with PathP cases), copy, copySet, copyExitEnd, wind into S1, windAdmit,
windExit, windAdmitEnd). The check row "admit without `M a ≡ 0`" is
test/check/membership-premise.lang, PATH_END ("the path at i1 is inr
unit, but the type needs inl unit"). The check row "a missing PathP
case" is test/check/membership-missing-case.lang, TYPE_MISMATCH ("elim
on Membership needs a motive, 3 cases and a scrutinee"). The eval rows:
copySet and copyExitEnd => `memberSet (fun b => inl unit)`, windAdmit
=> `fun i => loop i`, windExit => `fun i => loop ~i`, windAdmitEnd =>
`base`.

Mutants: one admission mutant for each new site (the rule_elim HIT class,
the elim case count, the path case type, the reduce_elim path branch, the
Membership splice and its premise p), with a "b" mutant for a row with a
second refusal class.

Result (S6, sessions 1 to 4, 2026-10-18). The gate `make -B check` is
GREEN, exit 0: examples 32, parse refusals 19, check refusals 164,
kit-only refusals 22, evals 85, lower refusals 16, build checks 33, and
the other counts equal to S5. The mutant loop kills 11 of 12 mutants
(docs/STATUS.md section S6). HI2 survives: its orbit-lang killer, the
fronttool row `hiit-wrong-head`, is an orbit-lang B4 row. That row is
not in a commit, thus it is not ported. The builders died in session 1
(Fable xhigh: 429; opus xhigh: `reasoning_extraction`), thus all four
sessions are by hand.

USER ruling (S6, 2026-10-18; AskUserQuestion, all recommended):
- Membership premises: relational (p, q, r above), not a kernel function
  update.
- Declaration site: the kernel splice after `def Agent`, not a
  parameterized domain family.
- O21 scope: uniform 1-HITs only.

Builder call (S7, 2026-10-18). Four parts. Kernel survey:
~/Documents/lend-lang-work/S7-survey-kernel.md. orbit-lang S7 (96c74e1,
orbit-lang PLAN section 8, "Builder call (S7, 2026-10-18)") has no code,
thus lend-lang builds the DAO prelude here and uses the orbit-lang B2 text
as the model. A later slice aligns the two after orbit-lang commits B2.

Part 0, the B4 rows (session 1, DONE). The patch is
`git -C ~/Documents/orbit-lang diff 8dd82a3 8b11846 -- test/` (4 files,
+39). The end context of test/check/expect.txt is different (the 2
Membership rows), thus the 2 new lines are appended. The gate is GREEN
(~/Documents/lend-lang-work/S7-p0-gate.log): check refusals 166, the other
counts equal to S6. HI2 is KILLED by the fronttool HIT_INDEX row
(~/Documents/lend-lang-work/S7-p0-HI2.log).

Part 1, the prelude (kernel text). check.c:3512-3530 makes act, Obj,
ChoiceRule, Aggregation, orbitProjection, Gov, IsFixedPointObj,
IsSelfConstituting, Config, amendPath and LendingLedger SIG_DEFINED, thus a
program def of one of them is REFUSE_SIG. The prelude is fixed source text
with origin ORIGIN_DOMAIN (the splice_after precedent, front.c:47-67),
spliced after the last program slot that it uses (Agent, Choice, G,
actChoice, D, Position). Obj has no M index (USER ruling S7).

```text
Obj                    := Agent -> Choice           -- the S4 domain of Orbit
act g x                := fun a => actChoice g (x a)
orbitProjection        := orbitProj                 -- S4
ChoiceRule X D         := X -> D
Constitution           := Σ (F : ChoiceRule Obj D), (g : G) (x : Obj) -> Path D (F (act g x)) (F x)
Aggregation F          := Σ (L : Orbit -> D), (x : Obj) -> Path D (L (orbitProj x)) (F x)
Gov L                  := fun x => L.1 (orbitProj x)
IsFixedPointObj F L    := (x : Obj) -> Path D (Gov L x) (F x)
IsSelfConstituting F L := IsFixedPointObj F L       -- O9: the ∃ is the Σ over L in Config
Config                 := Σ (M : Membership) Σ (F : ChoiceRule Obj D)
                          Σ (L : Aggregation F) (IsSelfConstituting F L × LendingLedger M)
```

Part 2, the ledger (kernel text). With 2, 0 and 1 as in S6 Part 3:

```text
State    := Sum Position (Sum Position Unit)        -- Active p, Forfeited p, Empty
Inv b s  := elim on s: Active _ => Path 2 b 1, Forfeited _ => Path 2 b 0, Empty => Unit
Ledger m := Σ (s : Agent -> State), (a : Agent) -> Inv (m a) (s a)
LendingLedger := elim (fun _ => Type 0) Ledger
                   (admit case: ua (admitEquiv ..)) (exit case: ua (exitEquiv ..))
```

- Inv is a case split into Path 2, thus no empty type is necessary:
  `Inv 1 (Forfeited p)` is `Path 2 1 0`.
- admitEquiv uses the premise r: for each b, elim on `r b`. The inl case
  (b ≡ a) swaps Forfeited p to Active p. The inr case keeps `s b`. The Inv
  field moves along q (or p) by subst. The equivalence is isoToEquiv with
  funExt and Σ paths. exitEquiv swaps Active p to Forfeited p. There is no
  decidable equality and no function update.
- Gaps (survey items 6 and 7): domain.lang has no isProp Unit, no isProp
  of a Path in 2 and no transport-cancel lemma. Part 2 adds them as kernel
  text. Ledger m must be Type 0 (ua and Glue are Type 0 only).

Part 3, amendPath and the laws (guest: examples/dao.lang,
examples/ledger.lang).
- amendPath is a Path Config c0 c1. The M face is the Membership path of
  the admits and exits. The F face is Gov L by definition. The L and sc
  faces come from transport-fill. The ledger face is the PathP over
  LendingLedger along the M face (ua).
- Laws: ⟦amend⟧ = Gov (refl); anonymity (L factors through orbitProj);
  the gov unit; self-constitution as a PathP over IsFixedPointObj. The
  uniqueness of descent (law 2) waits (USER ruling S7).
- Eval rows at closed agents (there is no regularity, survey item 8):
  transport along an exit at a gives Forfeited p at a, along an admit it
  gives Active p back, and at a different agent it keeps the state.

Gate rows (no new refusal code, survey item 4):
- "a non-invariant constitution": refl as the invariance proof of an F
  that actChoice changes, at Constitution. PATH_END (check.c:486).
- "a wrong unit": refl at the gov unit path for a G that is not F.
  PATH_END.
- "an exit that does not forfeit": an exit case that keeps Active p at a
  needs a term of `Path 2 0 1`. PATH_END or TYPE_MISMATCH (the row pins the
  code).
- "the ∃ of IsSelfConstituting as a Prop": a Config value whose sc field
  is a Trunc1 witness. TYPE_MISMATCH. A `Prop` use stays NAME_UNKNOWN
  (reserved, check.c:200-203).
- Lowering: no change. Σ, Glue and a neutral elim are REFUSE_LOWER, thus
  S7 has check and eval rows only.

Mutants: one admission mutant for each new site (the prelude splice, the
ledger splice, Inv at Forfeited, the admit swap branch), with a "b" mutant
for a row with a second refusal class.

USER ruling (S7, 2026-10-18; AskUserQuestion, all recommended):
- Obj index: `Obj := Agent -> Choice` (no M index; the S4 Orbit and its
  rows do not change). Config stays a Σ over Membership, and amendPath is
  a Path Config. The contract admits only member votes (O12 signatures).
- Law 2 uniqueness: it waits for the port of orbit-lang S7 B1 (square
  cases for squash constructors).
- Member permutation invariance (O8): S9, with orbit-lang O26. S7 checks
  the actChoice invariance only.

Result (S7, sessions 1 to 9, 2026-10-18). The gate `make -B check` is
GREEN, exit 0: examples 34, parse refusals 19, check refusals 170,
kit-only refusals 22, evals 88, lower refusals 16, build checks 33, and
the other counts equal to S6. The mutant loop kills 6 of 6 mutants
(docs/STATUS.md section S7). No builder stage completed (Fable xhigh:
429 in sessions 2, 3, 6 and 8; opus xhigh: `reasoning_extraction` in
sessions 6 and 8), thus the main loop made each part by hand. In session
5 the USER applied the session 4 changes to src/front/eval.c and
src/front/front.c (~/Documents/lend-lang-work/S7-p2-apply.sh), because
the auto mode classifier refused those edits.

Deviations from the builder call:
1. The Inv false cases are closed paths (`Path 2 0 1` at Active,
   `Path 2 1 0` at Forfeited). Thus isPropInv needs no `isSet 2`.
2. admit and exit share stepEquiv, because holdSwap is an involution.
3. The names are holdSwap, holdSwapSwap, holdSwap01 and holdSwap10.
   `swap` is a domain constructor (REFUSE_NAME).
4. LendingLedger is the plain ua form (no eta).
5. eval.c conv_values has the fast path
   `same = a == b || conv_loop(m, level, a, b);` (USER option 1,
   2026-10-18). Without it, the check of a closed Config value reaches
   the arena limit. Known limits: oomv-d (an inline Σ annotation gives
   two pointers) and oomv-a2 (`refl : Path (Membership -> Type 0) LL LL`).
6. The sc row uses `trunc1 T sc0`, because trunc1 takes the type
   explicitly.
7. amendPath is NOT POSSIBLE in this kernel. The ledger fill (transp
   over LendingLedger along exitOne) reaches the arena limit. The two
   smallest shapes are oomv-e (refl at the exitEquiv value, an Equiv
   conv) and oomv-f (`refl : Path Config cA cA`, M constant). The
   forward map alone checks, thus the limit is in the conv of the
   isEquiv part. examples/dao.lang has the laws amendGov, anon0, govUnit
   and scPath (all ok) and a comment that tells why it has no amendPath.
   O27 has the options. S7 has no kernel fix.

Builder call (S8, 2026-10-18). Survey:
~/Documents/lend-lang-work/S8-survey.md. O27 probe:
~/Documents/lend-lang-work/S8-o27-probe.md.

USER rulings (S8, 2026-10-18; AskUserQuestion, all recommended):
- O27: (b), then (c) if (b) does not clear (section 9, O27).
- Slice order: S8 = the port of orbit-lang B1 676c7a8 and B2 eafa1df,
  the O27 (b) probe, amendPath if (b) clears, and law 2 uniqueness. The
  ZK ledger moves to S9. The later slices move by one (the S7 ruling
  "S9" for O8 is now S10).
- B2 slots: a separate splice. `outcomeSet : isSet D` and `menu : D ->
  Constitution` with lend names. They do not join DAO_AFTER
  (front.c:69). The law 2 block splices after IsSelfConstituting and
  outcomeSet. The S7 rows and examples do not change. S8 does not port
  the orbit Config, Aggregation, Tally, aggregate, gov or amend (they
  clash in type with lend).

Part 0, O27 (b) probe (a copy only, no repo change). def_value
(eval.c:519) caches the unfolded body, and an applied def gives a new
value at each use, thus `a == b` (eval.c:3318) does not catch it. The
probe adds a Value field (core.h:227) with the kernel def index and the
argument vector, set when apply_value saturates a kernel def, and a test
at the top of conv_values (eval.c:3313): the same def index on both
sides, then conv the arguments, with a fallback to the unfold. Pass:
r12, a1, lFill, a2h and dh do not reach the arena limit, and the S7 gate
counts do not change. If all five pass, Part 3 ports it. If not, (c).
(S9 Part 5: the (b) head is given to every origin, eval.c def_value.)

Part 1, the B1 port (dependent elim for domain families). Sites: check.c
elim_family (:835), elim_at_domain (:960), rule_elim (:1020),
apply_known (:2569); eval.c reduce_elim (:733, :790). Take the B1
elim_on_family and remove the S6 uniform-family branch. Keep the lend
HIT_BOUNDARY "cannot eliminate" test (check.c:990-991). Probe the
Membership and Holding elims first: the ledger splice case arity must
not change. New files: examples/elim.lang, test/check/elim-block.lang,
elim-family-count.lang, elim-family-prop.lang, elim-motive-index.lang,
elim-path-case.lang; examples/kit.txt and test/check/kit.txt one row
each; test/fronttool.c (+31).

Part 2, the B2 slots and law 2. OP_SLOTS rows for outcomeSet and menu,
slot_ok, slot_shown, sig_row and the kernel_sig message. Law 2
uniqueness: an elim on Orbit (Quot) into `fun z => Path D (L z) (L' z)`.
The quot case is the factorization, the quotEq case is a PathP, and the
quotSquash case is a square by isSet D (outcomeSet).

Part 3 (only if Part 0 passes): the fast path in src/front/eval.c, and
amendPath (section 8, S7 Part 3 shape) in examples/dao.lang.

Part 0 result (2026-10-18): PASS. The conv head alone (try 1) clears
only lFill and dh. The conv head with the head-preserving quote_value
(try 2) clears all five shapes and the four controls. The copy gate
counts do not change. Thus Part 3 ports both parts of
~/Documents/lend-lang-work/S8-o27b.patch (src/front/eval.c,
src/front/core.h). Record: lend-lang-work/S8-o27b-probe.md.

Part 1 result (2026-10-18): PASS on try 2. Try 1 failed one lend S6 test
(test/fronttool.c, the overapplied Unit eliminator that ends in swOff):
the B1 elim_domain read it as a Switch elim. USER ruling (a): the B1
rule_elim keeps the lend S6 candidate fallback. If the motive has no
binder annotation and the scrutinee scan gives a family, rule_elim checks
the family reading on a trial Call. If that fails, it tries the Unit or
Sum position. If both fail, the family failure stays. The repo gate
`make -B check` is GREEN, exit 0: examples 35, check refusals 175,
kit-only refusals 22, evals 96, the other counts equal to S7. Record:
lend-lang-work/S8-p1-probe.md.

Part 2 result (2026-10-18): PASS. check.c has the SIGNATURE rows
outcomeSet and menu (SIG_SLOT) and OP_SLOTS (actChoice {G, Choice},
outcomeSet {D}, menu {D} with kernel Constitution). front.c splices the
law 2 block (orbitEq to descentUnique) after IsSelfConstituting and
outcomeSet, after the ledger splice. examples/dao.lang has outcomeSet, L1
and law2. The repo gate `make -B check` is GREEN, exit 0: check refusals
179, evals 98, the other counts equal to Part 1. Record:
lend-lang-work/S8-p2-build.md.

Part 3 probe (session 8, 2026-10-18): S8-o27b.patch applies to the repo
tree with no reject. Two problems:
1. amendPath is a SIG_DEFINED name (section 3). Since Part 2, check_sig
   refuses a program definition of it (REFUSE_SIG, the dao-redefine
   class).
2. With the fast path, the amend term (renamed) does not reach the arena
   limit, but it takes more than 20000000 eval steps (EVAL_FUEL). Bisect
   in the copy: the F face alone (amendGov @ i, M and the ledger constant)
   is ok, and `refl : Path Config cEnd cEnd` is ok. The M face with the
   ledger face (exitOne @ i, lFill @ i, F0 constant) is EVAL_FUEL. lFill
   alone is ok.
Record: lend-lang-work/S8-p3-probe.md. The repo has no Part 3 change.

USER rulings (S8 session 8, 2026-10-18; AskUserQuestion, all recommended):
- Part 3: find the conv that takes the fuel in the M face first, then a
  ruling. The fast-path port waits for that result.
- Name: the guest term is amendExit in examples/dao.lang. amendPath stays
  a reserved SIG_DEFINED name for a later kernel operator.

Part 3 result (sessions 9 to 11, 2026-10-18): PASS. All rows use one fuel
budget, the default of 20000000 eval steps. Diagnose (sessions 9 and 10,
record lend-lang-work/S8-s9-diag.md): with the M face, the Prod type of
the last Config field takes the steps. A Sigma with the same fields is
cheap (104342 steps). A Prod does not end at 200000000 steps. Root cause:
conv_loop (src/front/eval.c) compares the last argument of an op in its
loop step, and that step does not do the two fast tests of conv_values
(a == b, conv_head). Thus the compare of the ledger field of a Prod type
compares the unfolded Glue types of LendingLedger. A Sigma goes through
conv_binder and conv_values, thus it keeps the fast tests. USER ruling
(session 10): keep the head fast path on Prod. The probe showed that the
head is not lost (the two ledger arguments have the same head), but the
loop step does not test it. The fix has the same intent: after the switch
in conv_loop, if a == b or conv_head, conv_loop returns 1
(lend-lang-work/S8-s10-fix.patch, +7 lines). Steps after the fix: the M
face with the ledger face 98155, lFill 74593 (80555 before). The port
(session 11): S8-o27b.patch (eval.c, core.h), S8-s10-fix.patch, then
amendExit (lend-lang-work/S8-s11-resolve.py) in examples/dao.lang, the
check row amend-end (PATH_END) and 3 dao eval rows (amendHold0,
amendHold1, amendOther1). The repo gate `make -B check` is GREEN, exit 0:
examples 35, check refusals 180, evals 101, the other counts equal to
Part 2. Mutants: docs/STATUS.md section S8.

Kill rows (session 12, 2026-10-18): the USER rulings of session 11 add
two rows for the survivors m2 and m3. examples/dao.lang has a well-typed
`def menu : D -> Constitution` after c0 (m2). test/check/menu-early.lang
defines menu before the DAO prelude, thus the slot test refuses it
(REFUSE_SIG); with m3 it is REFUSE_REC. The Part 1 mutants (the motive
arity, the candidate fallback, the path-case IH end and the square-face
IH path) are all KILLED. The repo gate `make -B check` is GREEN, exit 0:
examples 35, check refusals 181, evals 101, the other counts equal to
Part 3.

Gate rows:
- B1: 5 check rows after elim-motive-dom: elim-family-count
  TYPE_MISMATCH, elim-path-case PATH_END, elim-motive-index
  TYPE_MISMATCH, elim-block TYPE_MISMATCH, elim-family-prop
  NAME_UNKNOWN. 8 eval rows: bagTwoCountE, replicateTwo, swapAt0,
  swapAt1, bagSwapE, quotThree, quotEqAt, hcompE.
- B2 with lend names: outcome-set-type and menu-type (a slot at a wrong
  type), dao-no-set (a law 2 use without outcomeSet: NAME_UNKNOWN),
  dao-redefine (a redefined lend SIG_DEFINED name: REFUSE_SIG). New lend
  dao eval rows (law 2 at the closed L0) in place of the orbit tallied,
  flippedTally, nextRule and amended.
- Counts: examples 34 + 1, check refusals 170 + 5 + the B2 rows, evals
  88 + 8 + the dao rows. amendPath rows only if Part 0 passes.

Mutants: one admission mutant for each new site (the elim_on_family
motive arity, the rule_elim candidate fallback (the Unit or Sum reading
after a failed family reading, USER ruling (a) S8), the path-case PathP
test, the squash square case, the
outcomeSet slot_ok, the menu slot_ok, the law 2 splice, and the
fast-path argument test if S8 ports it). make -B for each build.

Builder call (S9, 2026-10-19). Survey:
~/Documents/lend-lang-work/S9-survey.md. Findings: the port of
orbit-lang c3ab788 has 0 conflict hunks; `commit` can be prelude text
over the internal ecAdd and ecMul (a kernel prelude loads with
ORIGIN_DOMAIN, so REFUSE_KIT does not apply to it); the kernel has no
256-bit mod r (U256 ops are add, sub, le, eq; Nat is a uint64); zkEquiv
as a guest theorem needs isSet Commitment and set-quotient
effectiveness, and neither exists at HEAD.

USER rulings (S9, 2026-10-19; AskUserQuestion, all recommended):
- Part order: the port of orbit-lang c3ab788 first, then the host
  Pedersen and Sigma prover with a known-answer fixture, then the zk
  prelude text and the guest terms, then the lowering and the chain
  rows. Each part has its own gate.
- Generator H (O13): try-and-increment hash-to-curve. x = keccak256(tag
  || counter) mod p with tag "lend-lang/H" and the counter from 0, until
  x^3 + 3 is a square; y = the smaller root. The host derives H once,
  the prelude carries the constant, and docs/VALIDATION.md cites the tag
  and the counter. Nobody knows log_G H.
- Range proof (O15): a Schnorr opening proof plus a 32-bit decomposition
  with one CDS OR proof per bit. The split `e0 + e1 = e (mod r)` needs
  one new internal op u256AddMod, lowered to the EVM ADDMOD opcode.
  Amounts are 32-bit words.
- zkEquiv (O14): on the guest side Commitment := Trunc0 (Prod U256
  U256), a set by squash0. Effectiveness of the set quotient is a guest
  theorem by ua (S5) in its own part (Part 5). If Part 5 does not clear
  in two tries, zkEquiv is a declared equivalence (O14 (b)) with a note
  here, and the theorem moves to S11.

Defaults (S9, not ruled; the USER can change them):
- Commitment shape: one plain Pedersen commitment per component of
  Position = (collateral, debt). Commitment is the product group G1 x
  G1, and the step rows of section 6 hold per component. (The other
  shape: a vector Pedersen with a third generator.)
- Prover home: a C fixture over bn254.h (test/zktool.c, the fronttool
  pattern) prints the known-answer words. A `langc prove` verb waits for
  S10.
- Fiat-Shamir: e is a chain of the 2-word hash over the words of G, H, C
  and A. The contract uses e raw as the ecMul scalar (0x07 reduces mod
  r). The prover reduces e mod r in Fr.
- A step (borrow, repay, liquidate) is an entry, a state change. Paths
  stay with admit and exit (S7) and with the transport along Loan. The
  section 6 sentence "each step is a path in LendingLedger M" reads so.
- Nonces: deterministic, k = keccak256(secret || transcript) reduced
  mod r. The host has no CSPRNG.
- Size to word: an evaluator-only fold in the prelude (Size -> U256)
  feeds commit. The lowerer does not see it; the contract sees
  Commitment words only. The section 4 rule stays a later slice.

Part 1, the port of orbit-lang c3ab788 (closed BN254 ecAdd and ecMul in
the host). 10 files, 0 conflict hunks (survey section 1). New files:
src/front/bn254.c, src/front/bn254.h, examples/bn254.lang (kit.txt
row). Makefile FRONT (:6) gets bn254.c. eval.c: `#include
"front/bn254.h"`, closed_point and ec_op after `opaque` (eval.c:1415),
and the OP_EC_ADD and OP_EC_MUL cases of reduce_op (eval.c:3123-3126)
call ec_op. Moved rows, as orbit: test/lower/path-word.lang takes `(x :
U256)`; test/precompile.sh addBad takes `2u` (`call revert addBad 2u`)
and the file gains 6 chain rows (3G, (r - 1)G, rG); test/eval/expect.txt
twoG becomes the word pair, plus 10 bn254 rows. Check after the build:
test/lowertool.c:278-282 use a closed bad point, which now traps in the
evaluator (survey section 6).

Part 2, the host Pedersen and Sigma code. src/front/pedersen.c and .h
over bn254.h: the H derivation (the ruling above), commit, the Schnorr
opening prover and verifier, the CDS OR bit prover and verifier, the
range prover and verifier over 32 bits, and the hash chain.
test/zktool.c derives H, prints the known-answer words (commit of a
fixed (p, r), a Schnorr vector, bit vectors for 0 and 1, a 32-bit range
vector) and checks each verifier on them and on the negatives; `make
check` runs it. Part 2 gate: the zktool rows pass, the repo counts equal
Part 1.

Part 3, the kernel text. (a) The op u256AddMod: a check.c SIGNATURE row
after u256Eq (:182; arity 3, VIS_INTERNAL), core.h OP_U256_ADDMOD, the
eval.c name and arity tables (:25, :50) and a reduce_op case near
OP_U256_ADD (:2067), ir.h IR_OP_ADDMOD after IR_OP_MOD (:51), the
lower.c op table (:399-402), the asm.c and evm.c opcode. (b) A zk
prelude in front.c (zk_source, spliced after Position and Ledger by
splice_after_all, front.c:303): Commitment := Trunc0 (Prod U256 U256)
with a word-pair view for the contract; G = pair 1u 2u; H = the Part 2
constant; Blind := U256; Opening := Prod Position Blind; commit (two
components); PrivateFace := Quot Opening R with R o o' := Path
Commitment (commit o) (commit o'); PublicFace := Sigma (c : Commitment)
(Trunc1 (fiber commit c)); Loan i := Glue; PrivateLoan c; ZKProof as
the word tuple (A, s1, s2) plus the 32 bit proofs; the unrolled
verifier over hash, ecAdd, ecMul, u256AddMod and u256Eq. (c) The steps
borrow, repay and liquidate as entries on the S7 ledger (Holding Active,
Forfeited, Empty; Inv): a non-member or a Forfeited position reverts;
liquidate after the grace window (O17, `now`). examples/zk.lang
(kit.txt row) carries the Part 2 known-answer values. Part 3 status
(2026-10-19): DONE. The entries are zkBorrow, zkRepay and zkLiquidate
(borrow, repay and liquidate are design-signature names); the verifier is a
fold over the bit proofs, not unrolled; no Loan or PrivateLoan (the Equiv of
Part 5). Part 3 gate after staged review: examples 37, check refusals 184,
evals 132, zk 33, the
other counts equal Part 2, exit 0.

Staged review (2026-10-19): deterministic nonce derivation separates
Schnorr, bit and range domains, preventing blind recovery when two proof
protocols use the same opening. The prelude requires exactly 32 range
bits and canonical proof scalars below r, matching the host verifier.
The refreshed vectors and regression evidence are in docs/VALIDATION.md.

Part 4, the lowering and chain rows. The verifier lowers to STATICCALL
0x06 and 0x07 (lower_curve, lower.c:583-604) and ADDMOD. test/zk.sh
(the test/precompile.sh `call` shape) checks the commit words, a
Schnorr proof, a bit proof and a 32-bit range proof on the chain, and
the reverts (wrong opening, range overflow, non-member, Forfeited, a
point not on the curve). S10 reads the gas numbers from here. Part 4
status (2026-10-19): DONE. The range proof on the chain is 32 zkRangeBit
entries (10 words each; the state holds the Horner accumulator accx, accy
and the count bits; rb31 first) and zkRangeDone (the prelude owns the name
zkRangeEnd). The views zkCommitX, zkCommitY, zkSchnorrView and zkBitView
take the proof words as parameters, so the curve ops stay open and lower
to the 0x06 and 0x07 precompiles. The selector helper of test/evmchain.sh
(and test/dispatch.sh) needed `od -v`: od folds a repeated 16-byte line
into `*`, and `uint256,uint256,` repeats on that boundary from six
arguments up, so each 6-word and 10-word call got a non-hex selector and
an empty call data. Part 4 gate: test/zk.sh 56 steps, 0 failures; builds
4; the other counts equal Part 3, exit 0. Part 5 gate: evals 139,
check refusals 186; the other counts equal Part 4, exit 0. DONE
2026-10-19: the guest theorem is zkFaceEquiv (the kernel reserves
zkEquiv for the design signature) under the O27 (b) head for every
origin.

Part 5, zkEquiv as a guest theorem (the O14 ruling). Forward: elim on
the Quot; the quotSquash case uses isSet PublicFace, from isSet
Commitment (squash0) and the Sigma of a set and a prop. Backward: elim
on Trunc1 into the prop Sigma (q : PrivateFace) (Path Commitment
(commit' q) c), which needs effectiveness of the set quotient (by ua,
a code family over Quot) (S9 Part 5: it needs injectivity of commit',
the quotEq constructor; no effectiveness and no ua code family).
isoToEquiv (domain.lang:107) closes it. If Part 5 does not clear in
two tries, the fallback above.

Gate rows:
- Part 1: the orbit rows of c3ab788 (survey section 1): 10 bn254 eval
  rows, 1 moved eval row, 6 precompile chain rows, 1 moved precompile
  row, path-word moved. Counts: examples 35 + 1, evals 101 + 10,
  precompile 18 + 6.
- Part 2: test/zktool.c rows (the H counter and words, the commit
  known answer, Schnorr ok, bit 0 ok, bit 1 ok, range ok; the
  negatives: a wrong s, a bit value 2, a point off the curve).
- Part 3: examples/zk.lang eval rows (commitKA => pair, schnorrOk => 1,
  schnorrBad => 0, bitOk0, bitOk1, bitBad => 0, rangeOk, rangeBad,
  commitBad 2 => trap); check rows: zk-redefine (a redefined zk prelude
  name: REFUSE_SIG), zk-guest-word (a guest word literal in a proof:
  REFUSE_KIT), addmod-guest (u256AddMod in guest mode: REFUSE_KIT).
- Part 4: test/zk.sh chain rows (7 known-answer views, the 5 reverts,
  the 32-step range chain and zkRangeDone, the flow rows zkRepay,
  zkLiquidate at 8 and 9, a Forfeited zkBorrow): 56 steps, 0 failures.
- Part 5: eval rows for zkEquiv at the closed known-answer opening
  (forward, backward, both round trips).

Mutants (one at a time, make -B for each build, restore after each):
wrong opening (drop the e·C term of the verifier: the Schnorr row and
the chain row fail), range overflow (skip one OR check of the unrolled
verifier: rangeBad passes, so its row fails), non-member step (remove
the member test of borrow: the revert row fails), a step on a Forfeited
position (remove the Holding case split: the revert row fails), a point
not on the curve (g1_from_words without g1_on_curve: commitBad and the
bn254 addBad row fail), plus one admission mutant for each new kernel
site (the u256AddMod reduce, the zk splice, the Trunc0 set case of
Part 5).

## 9. Open items

USER ruling (S1, 2026-10-17): every default below is RULED. Where an item
has options, (a) is the ruling. O20 is a USER ruling from the request.

- O1 Name: lend-lang, after the lending ledger.
- O2 Kernel route: (a) the orbit-lang lineage (section 5: port 7734007,
  the 3a1ba76 mechanism, 9e28399, then orbit-lang S5). (b) port sealed-lang
  again as orbit-lang S2 did (route Q, sealed-lang bc8e168 first), then
  redo S3 and S4. (c) the zkpolicy-lang lineage (tcc-wasm kernel, three-way
  merges, the EVM erasure ported again).
- O3 Base span: (a) strict: kit word types are lowering carriers only. (b)
  admit Nat, U256, Addr, Flag and Map as guest types. (b) adds base types
  that the request does not admit.
- O4 Signature kinds: the slot, defined and primitive split of section 3.
- O5 Member sets: (a) Agent is a finite slot type and a member set is
  `Agent → 2`. (b) a finite-set QIT (lists modulo swap and duplicates).
- O6 Membership paths: (a) two path constructors admit and exit with the
  premises of section 3, and no 2-cells; `exit a ∙ admit a` transports to
  the identity on the ledger by a guest theorem. (b) exit is a definition,
  `sym (admit a)`.
- O7 LendingLedger: the State reading of section 6 (exit forfeits by
  transport, readmission restores).
- O8 Group: as orbit-lang O5 (a): G may act on choices (the reference
  neutrality) and by member permutation; the run time count forgets
  members, so each constitution must be invariant under member
  permutation, and the compiler checks this.
- O9 Self-constitution: the design defines `IsSelfConstituting F` with
  `∃ L`, but Config applies it to L. Reading: the Σ over L in Config is the
  witness, and the field is `IsFixedPointObj L`. The ∃ is never a Prop and
  never a truncation (O20).
- O10 ChoiceRule and D: a functor between discrete categories is a
  function. D is a slot, and the default examples take D := lending terms
  (collateral ratio num/den, rate num/den).
- O11 amendPath: built from the Membership path of the admits and exits,
  `Gov L` as the new constitution, and transport-fill for L', sc' and LS'.
  The constitution face is `Gov L` by definition.
- O12 Votes: the design has no vote operation. (a) a vote is one
  coordinate of the configuration in `proposal`; the `vote` entry lowers
  that argument; the signature is a bLSAG ring signature over the members
  (orbit-lang O10), so the contract keeps only the orbit. (b) one
  transaction carries a whole configuration signed by all members (no
  anonymity).
- O13 Commitment: (a) Pedersen over BN254 G1. (b) a keccak256 commitment
  (no homomorphism, so the design's monoid law fails). RULED S9
  (2026-10-19): (a); H by try-and-increment hash-to-curve from the tag
  "lend-lang/H" (section 8, S9 rulings); one commitment per component of
  Position (S9 default).
- O14 PrivateLoan and zkEquiv: (a) the first isomorphism reading of
  section 6. (b) a primitive equivalence that the host checks on test
  vectors only (orbit-lang O11 style). RULED S9 (2026-10-19): (a), with
  Commitment := Trunc0 (Prod U256 U256) on the guest side and quotient
  effectiveness as a guest theorem (section 8, S9 Part 5); (b) is the
  fallback if Part 5 does not clear. S10 Part 1 (2026-10-20): the guest
  definition is zkPrivateLoan; the signature name PrivateLoan stays
  reserved to the kernel (REFUSE_SIG), as zkEquiv became zkFaceEquiv.
- O15 Proof system: (a) Sigma protocols with bit-decomposition range
  proofs (0x06, 0x07). (b) Groth16 through 0x08 with fixture proofs and an
  out-of-tree prover. (c) Bulletproofs (short proofs, high verify gas).
  RULED S9 (2026-10-19): (a), a 32-bit decomposition with one CDS OR
  proof per bit; the challenge split uses a new internal op u256AddMod
  (the EVM ADDMOD opcode), which section 6 did not list.
- O16 Interest: the rate num/den accrues per epoch; each step proves
  `debt' = debt + ceil(debt · num / den)` with a range proof on the
  remainder. Epoch length is a program constant.
- O17 Liquidation: after an amendment that changes the terms, each holder
  posts a fresh health proof within a grace window (a block count in the
  program). After the window, `liquidate` makes a position without one
  Forfeited, the same state as an exit.
- O18 Assets: (a) none: a private credit ledger in one abstract unit; the
  design admits no deposit or withdraw. (b) shielded deposit and withdraw
  as kit-mode entries, outside the design span (needs a USER ruling).
- O19 Member access: (a) each loan step carries a Schnorr signature by the
  agent key: the identity is public, the amounts are private. (b) a bLSAG
  ring signature and a per-step nullifier (anonymous borrowers; positions
  keyed by a pseudonym).
- O20 Proof irrelevance: PROHIBITED (USER ruling 2026-10-17, section 1).
- O21 Dependent elimination on a HIT: the cases of path constructors are
  PathP over the motive. LendingLedger over Membership (S7) and law
  uniqueness need it. orbit-lang O21 has the same item; S6 builds it here.
- O22 Chains: Ethereum mainnet has 0x06, 0x07 and 0x08. Telcoin Network:
  confirm the precompile set and the EIP-170 limit from the chain spec or
  an RPC probe (orbit-lang O16). The repo gate runs only the kit EVM runner.
- O23 Regimes: the design says amendments inherit the three UAT regimes.
  Default: the orbit-lang O8 and O9 reading (a three-point coproduct the
  compiler computes from the menu), at S11.
- O24 Scope of the claim: lend-lang proves the laws on the discrete
  quotient reading (U/Characterization.lean:81). Hiding, binding, soundness
  and zero knowledge are assumptions in docs/VALIDATION.md.
- O25 orbit-lang: a separate repo. lend-lang ports orbit-lang kernel
  commits; it does not change orbit-lang.
- O26 Off-chain commands: `langc prove` writes the calldata of vote,
  borrow and repay; `langc run` evaluates steps and amendments in the host.
- O27 Conv limit (S7; RULED S8, 2026-10-18: (b), then (c) if (b) does
  not clear): the conv OOM family oomv-a2, oomv-d, oomv-e and oomv-f
  blocks amendPath (section 8, "Result (S7, ...)", deviation 7). Options:
  (a) the CONV_CLOS same_closure test
  (~/Documents/lend-lang-work/S7-p2-scratch/oom-patch.py, block 2);
  (b) a conv fast path for the Equiv or the Glue of a ledger step:
  compare the kernel def and its arguments before the unfold; (c) keep
  amendPath out, with the laws only. S8 probe on a patched copy of
  7d6a401 (~/Documents/lend-lang-work/S8-o27-probe.md): (a) clears only
  oomv-d. oomv-a2, oomv-e, oomv-f and the amendPath ledger fill stay at
  the 1 GB arena limit. Ruling: S8 probes (b) in a copy against the five
  shapes (r12, a1, lFill, a2h, dh). If (b) clears all five, S8 ports it
  and adds amendPath. If not, (c): amendPath stays out.
  Part 0 PASS (2026-10-18): (b) clears all five shapes only with the
  head-preserving quote_value (S8-o27b-probe.md). S8 Part 3 ports it.
