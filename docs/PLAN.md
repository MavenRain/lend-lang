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
| S4 | port orbit-lang 9e28399: Orbit after actChoice (choices only; the M index waits for Obj and act), elim on Sum and Unit: DONE 2026-10-18 | Orbit fold without isSet, wrong boundary, Orbit path at run time |
| S5 | Glue, ua, uaBeta, hcomp at Glue and U (port of orbit-lang a12973a): DONE 2026-10-18 | Glue boundary mismatch, ua on a non-equivalence |
| S6 | HIITs, QIITs, dependent elim on a HIT with PathP cases; Membership as a kernel HIT: DONE 2026-10-18 | wrong index in a path constructor, inductive-inductive order, a missing PathP case, admit without `M a ≡ 0` |
| S7 | DAO prelude: Obj, act, ChoiceRule, Aggregation, Gov, IsFixedPointObj, Config, amendPath; LendingLedger over Membership by ua: DONE 2026-10-18, without amendPath (O27) | a non-invariant constitution, a wrong unit, an exit that does not forfeit, the ∃ of IsSelfConstituting as a Prop |
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
- O27 Conv limit (S7; NOT RULED, the USER rules it at the start of S8):
  the conv OOM family oomv-a2, oomv-d, oomv-e and oomv-f blocks
  amendPath (section 8, "Result (S7, ...)", deviation 7). Options, no
  ruling: (a) the CONV_CLOS same_closure test
  (~/Documents/lend-lang-work/S7-p2-scratch/oom-patch.py, block 2), the
  known fix candidate; (b) a conv fast path for the Equiv or the Glue of
  a ledger step: compare the kernel def and its arguments before the
  unfold (a hypothesis, not tested); (c) keep amendPath out, with the
  laws only.
