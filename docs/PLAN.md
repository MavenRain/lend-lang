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
| S5 | orbit-lang S5 (coin-lang 44c917f, 91d7091, 34acff5) | when the USER commits it | Fin, equivalences, Glue, ua, uaBeta, hcomp at Glue and U |
| S6 | new | (design reference: zkpolicy-lang a1780a9, f176ac6) | HIITs, QIITs, dependent elim on a HIT with PathP cases (O21) |

S6 is new work in this lineage, as in orbit-lang. If orbit-lang reaches S6
first, lend-lang ports it.

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

zkEquiv needs no cryptographic assumption, so transport along `Loan`
computes. The holder keeps a representative opening. The contract sees
only the public face.

### Proofs and steps

Default proof system (O15): Sigma protocols over the Pedersen group, with a
keccak256 Fiat-Shamir challenge. A Schnorr proof shows knowledge of an
opening. A range check uses a bit decomposition with one OR proof per bit.
The verifier needs only ecAdd (0x06) and ecMul (0x07). There is no trusted
setup and no circuit compiler, and the TinyCC host runs the prover. Cost:
about 4 ecMul per range bit; the S9 gas table gives the numbers.

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
| S4 | port orbit-lang 9e28399: Orbit M after act, elim on Sum and Unit | Orbit fold without isSet, wrong boundary, Orbit path at run time |
| S5 | Glue, ua, uaBeta, hcomp at Glue and U (port of orbit-lang S5) | Glue boundary mismatch, ua on a non-equivalence |
| S6 | HIITs, QIITs, dependent elim on a HIT with PathP cases; Membership as a kernel HIT | wrong index in a path constructor, inductive-inductive order, a missing PathP case, admit without `M a ≡ 0` |
| S7 | DAO prelude: Obj, act, ChoiceRule, Aggregation, Gov, IsFixedPointObj, Config, amendPath; LendingLedger over Membership by ua | a non-invariant constitution, a wrong unit, an exit that does not forfeit, the ∃ of IsSelfConstituting as a Prop |
| S8 | ZK ledger: Pedersen on BN254 in C, Sigma-protocol verifier lowering, zkEquiv, PrivateLoan, borrow, repay, liquidate | known-answer vectors; mutants: wrong opening, range overflow, non-member step, step on a Forfeited position, point not on the curve |
| S9 | contract end to end: deploy, vote, amend, borrow, repay, liquidate; host and EVM differential; gas table; chain probe | differential rows, revert rows, EIP-170 and EIP-3860 size limits |
| S10 | regimes (optional, O23), docs, final gate | regime rows |

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
  (no homomorphism, so the design's monoid law fails).
- O14 PrivateLoan and zkEquiv: (a) the first isomorphism reading of
  section 6. (b) a primitive equivalence that the host checks on test
  vectors only (orbit-lang O11 style).
- O15 Proof system: (a) Sigma protocols with bit-decomposition range
  proofs (0x06, 0x07). (b) Groth16 through 0x08 with fixture proofs and an
  out-of-tree prover. (c) Bulletproofs (short proofs, high verify gas).
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
  compiler computes from the menu), at S10.
- O24 Scope of the claim: lend-lang proves the laws on the discrete
  quotient reading (U/Characterization.lean:81). Hiding, binding, soundness
  and zero knowledge are assumptions in docs/VALIDATION.md.
- O25 orbit-lang: a separate repo. lend-lang ports orbit-lang kernel
  commits; it does not change orbit-lang.
- O26 Off-chain commands: `langc prove` writes the calldata of vote,
  borrow and repay; `langc run` evaluates steps and amendments in the host.
