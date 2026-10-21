# lend-lang status

> Write here: one paragraph. Say which milestone the build implements, and
> which milestone it does not complete yet.

## Implemented

> Write here: one item for each feature that works. Name the former (F1 to
> F15), the core type or the core operation.

- <feature>.
- The regimes (S13, O23 (a)): examples/regimes.lang. The guest term
  `aggregate` (elim on Orbit) gives the outcome of a constitution at a
  tally. `regimeOf` counts the self constituting phases of a menu at a
  tally: AI (none), AD (one) or SI (more than one). The laws paraLaw,
  forkLaw and noneLaw hold by refl. The Empty family
  (domain/domain.lang) gives Not and exfalso, and noSelfCon (no self
  constitution without an aggregation).

## Remaining M0 work

> Write here: one numbered item for each missing feature. Say what each item
> needs first.

1. <work item>.

## Known limits

> Write here: one item for each limit of the current build, with its cause
> (a fuel budget, a host limit, a stack size).

- <limit>.
- The regimes are guest terms. The checker and `langc eval` read
  examples/regimes.lang (guest mode: examples/kit.txt has no line for
  it). No build, lower or contract step uses a regime. The example has
  2 agents, 3 phases and fixed menus (beta 1/1, 2/1, 1/2 and the cycle).
  Cause: the S13 scope (Q-S13-1 (a): a guest `aggregate` by elim on
  Orbit).
- Empty has one constructor (emptyLoop) and no base case, not zero
  constructors. Cause: the parser needs a constructor in a family.

## Internal boundaries

> Write here: one paragraph for each module group. Say which module holds
> which job, which tests check which tables, and which types a program
> cannot use.

## S2

Slice S2 (the port of orbit-lang 7734007, PLAN section 8) is complete on
commit d4fdb2e plus the S2 changes. orbit-lang 7734007 ports the
sealed-lang S2 to S4 kernel (route Q, orbit-lang PLAN section 8). lend-lang
applies one patch: the orbit-lang diff 6cd1104 to 7734007 without
docs/PLAN.md, README.md and design, with `git apply --3way`. The apply has
0 conflict hunks (PLAN section 8 "Builder call (S2, 2026-10-17)"). The gate
`make -B check` is GREEN, exit 0, with the tcc `-Wall -Werror` builds.

| Count | S1 | S2 | orbit-lang S2 | Equal |
|---|---|---|---|---|
| examples | 19 | 24 | 24 | yes |
| parse refusals | 13 | 19 | 19 | yes |
| check refusals | 57 | 91 | 91 | yes |
| evals | 55 | 61 | 61 | yes |
| lower refusals | 11 | 14 | 14 | yes |
| build checks | 28 | 31 | 31 | yes |
| precompile steps | 0 | 18 | 18 | yes |
| path regressions | none | passed | passed | yes |

The other counts do not change, and they are equal to the orbit-lang S2
counts: runs 5, run refusals 10, asmtool 8 of 8, builds 3, dispatch 20,
evm steps 43, mock 13 rows, diff 15 rows, emit 24, and the review, domain,
fuel, output, lower and hashing regressions. No count differs, thus the
reason for each change is the orbit-lang S2 reason (orbit-lang 7734007
docs/STATUS.md section S2): the sealed-lang S2 to S4 rows come in, the
sample domain goes, and 12 Prop-like refusal rows come in.

Prop-like refusals (USER ruling, O20: no proof irrelevance). The port
brings these 12 rows:

| Row | Form | Code |
|---|---|---|
| check `prop-type`, `sprop-type` | `Prop`, `SProp` as a type | NAME_UNKNOWN |
| check `prop-def`, `sprop-def` | `Prop`, `SProp` as a defined name | REFUSE_NAME |
| check `trunc1-refl`, `trunc0-refl`, `isprop-refl` | refl between two non-convertible elements of Trunc1 A, Trunc0 A and an isProp-typed type | PATH_END |
| check `loop-twice-not-loop` | a path of paths by refl between two different loops (K) | PATH_UIP |
| parse `irrelevant-dot`, `irrelevant-brace`, `irrelevant-attr` | irrelevant binder markers `.(x : Nat)`, `{x : Nat}` and the attribute `#[irrelevant]` | LEX_CHAR |
| parse `irrelevant-at` | irrelevant binder marker `(@irrelevant x : Nat)` | PARSE_EXPECT |

The ported rows `path-uip`, `path-k` and `loop-not-refl` also refuse UIP
and K (PATH_UIP, rule R9).

orbit-lang cites. `rg -n -i orbit` finds these cites of orbit-lang as the
port source or as the source of the Prop ruling: src/front/check.c:171,
test/check/prop-type.lang:1, test/check/sprop-type.lang:1,
domain/domain.lang:2, three section titles in formers/tcc-evm-contract.md
and in docs/host/README.md ("ported in orbit-lang S2"), and the runner cite
below. lend-lang has the same ruling (O20). These cites stay, thus src,
test and domain stay equal to the orbit-lang 7734007 trees for the S3
port. docs/host/README.md lines 3 and 27 named orbit-lang as this repo;
S2 changes them to lend-lang. The other hits are in design/DESIGN.md (the
design text, "voter-orbit"), README.md and docs/PLAN.md (the port route).

Mutants, one at a time (runner `lend-lang-work/S2-mutants.sh`, log
`lend-lang-work/S2-mutants.log`). The runner is a copy of
`orbit-lang-work/S2-mutants.sh` (M1 to M5, M3b, M4b) plus M6 to M11, for
the refusal classes that orbit-lang S2 did not mutate. The runner saves
each file with cp, mutates it with awk, moves its mtime 2 s back, rebuilds
with `make -B build`, refuses a stale `build/langc`, runs the rows and
restores the file with cp. After the loop, check.c, lexer.c, evm.c and
lower.c are equal to their saves and to the index, and the final
`make -B build` exits 0.

| Mutant | Site | Edit | Killer | Verdict |
|---|---|---|---|---|
| M1 Prop names not reserved | src/front/check.c:174 | RESERVED: `"Prop", "SProp"` removed | check `prop-def`, `sprop-def`: want REFUSE_NAME, got exit 0 | KILLED |
| M2 braces read as parentheses | src/front/lexer.c:219 | `{` and `}` lex as TOK_LPAREN and TOK_RPAREN | parse `irrelevant-brace`: want LEX_CHAR, got exit 0 | KILLED |
| M3 PATH_END fails without a diagnostic | src/front/check.c:457 | end_fail: the PATH_END branch returns 0 | `trunc1-refl`, `trunc0-refl`, `isprop-refl`: want PATH_END, got exit 1 with no message | KILLED (diagnostic loss) |
| M3b PATH_END end check passes | src/front/check.c:457 | end_fail: the PATH_END branch returns 1 | `trunc1-refl`, `trunc0-refl`, `isprop-refl`: want PATH_END, got exit 0 | KILLED |
| M4 PATH_UIP fails without a diagnostic | src/front/check.c:456 | end_fail: the PATH_UIP branch returns 0 | `loop-twice-not-loop`, `path-uip`, `path-k`, `loop-not-refl`: want PATH_UIP, got exit 1 with no message | KILLED (diagnostic loss) |
| M4b PATH_UIP end check passes | src/front/check.c:456 | end_fail: the PATH_UIP branch returns 1 | `loop-twice-not-loop`, `path-uip`, `path-k`, `loop-not-refl`: want PATH_UIP, got exit 0 | KILLED |
| M5 STATICCALL result not tested | src/evm.c:372 | the ISZERO after STATICCALL replaced by POP, the jump to revert removed | precompile `addBad` (EIP-196, a point not on the curve): want revert, got out 0 | KILLED |
| M6 Prop and SProp resolve as Unit | src/front/check.c:1841 | the builtin lookup gives Unit for a name that contains `Prop` | check `prop-type`: want NAME_UNKNOWN, got TYPE_MISMATCH; `sprop-type`: want NAME_UNKNOWN, got exit 0 | KILLED |
| M7 dot is white space | src/front/lexer.c:108 | skip_trivia skips `.` | parse `irrelevant-dot`: want LEX_CHAR, got PARSE_EXPECT | KILLED |
| M8 `#` starts a comment | src/front/lexer.c:109 | skip_trivia reads `#` to the end of the line | parse `irrelevant-attr`: want LEX_CHAR, got exit 0 | KILLED |
| M9 `@irrelevant` dropped | src/front/lexer.c:107 | skip_trivia skips the 11 bytes `@irrelevant` | parse `irrelevant-at`: want PARSE_EXPECT, got exit 0 | KILLED |
| M10 ecPairing to address 7 | src/lower.c:602 | lower_curve: ecPairing calls 0x07 | precompile `pairOk` (EIP-197): want out 1, got out 237bfc65...; `pairNil`: want out 1, got out 0 | KILLED |
| M11 ecAdd to address 7 | src/lower.c:602 | lower_curve: ecAdd calls 0x07 | precompile `addX`, `addY` (EIP-196, 2G): want 2G, got out 1 and out 2 | KILLED |

Score: 13 of 13 killed. M1, M2, M3b, M4b, M5, M8, M9 and M6
(`sprop-type`) make the refused form pass, so they test admission, not
only the diagnostic.

Builder. S2 did not call a builder. The port is one `git apply` with 0
conflict hunks and two lines of docs/host/README.md, and the mutant loop
is a script, thus the main loop did the work.

## S3

Slice S3 (base-span pruning, PLAN section 4 and section 8) is complete on
commit 2449b67 plus the S3 changes. The mechanism is the mechanism of
orbit-lang 3a1ba76: a visibility tag on the BUILTINS, the origin
ORIGIN_KIT for the contract prelude, and a check mode. Guest mode is the
default and refuses a kit name, a Nat or word literal and a `state` or
`event` declaration (REFUSE_KIT). `langc --kit` selects the kit mode. The
port is one patch, `git apply --3way`, with 1 conflict file
(docs/VALIDATION.md, 2 prose hunks) and 0 code conflicts (PLAN section 8
"Builder call (S3, 2026-10-17)"). lend-lang keeps one delta that
orbit-lang does not have: the USER review fix of 2449b67 (`conv_system`,
`conv_branches`, `mark_intervals` in src/front/eval.c and 5 rows in
test/pathtool.c). The table SIGNATURE in `src/front/check.c` holds the 18
types and 13 operations of the lend-lang design signature (REFUSE_SIG).
The gate `make -B check` is GREEN, exit 0 (log `lend-lang-work/S3-gate.log`).

| Count | S2 (2449b67) | S3 | orbit-lang S3 | Reason |
|---|---|---|---|---|
| examples | 24 | 25 | 25 | `signature` (guest): the six slots from formers and `actChoice` at its slot type (the Z2 flip) |
| parse refusals | 19 | 19 | 19 | no change (the 17 kit rows run under `--kit`) |
| check refusals | 91 | 133 | 119 | 22 REFUSE_KIT rows (as orbit-lang), 16 REFUSE_SIG rows and 4 Prop rows |
| kit-only refusals | none | 22 | 22 | each REFUSE_KIT row gives `ok` under `--kit` |
| evals | 61 | 62 | 62 | `signature flipped => inr unit` |

Check refusals 133 against 119: orbit-lang S3 has 119 rows, 6 of them its
REFUSE_SIG rows. lend-lang removes the 6 orbit-lang rows (-6), adds 16
lend-lang REFUSE_SIG rows (+16) and adds 4 Prop rows (+4): 119 - 6 + 16 +
4 = 133. The other counts do not change and are equal to S2: runs 5, run
refusals 10, asmtool 8 of 8, builds 3, lower refusals 14, build checks
31, dispatch 20, evm steps 43, precompile steps 18, mock 13 rows, diff 15
rows, emit 24, and the review, domain, path, fuel, output, lower and
hashing regressions.

Rows per mode. A kit row is listed in a `kit.txt` file and runs under
`--kit`.

| Gate | Kit rows | Guest rows |
|---|---|---|
| examples (`examples/kit.txt`) | 24 | 1 (`signature`) |
| check refusals (`test/check/kit.txt`) | 82 (the orbit-lang 80, `prop-def-kit`, `sprop-def-kit`) | 51 (the orbit-lang 39, -6 orbit-lang REFUSE_SIG rows, +16 lend-lang REFUSE_SIG rows, `prop-type-guest`, `sprop-type-guest`) |
| parse refusals (`test/parse/kit.txt`) | 17 | 2 |
| evals | 61 | 1 (`signature flipped`) |

The signature (PLAN section 3, O4). A slot is a program definition at
its slot type. A defined or a primitive name is reserved until a later
slice gives it.

| Kind | Types | Operations |
|---|---|---|
| slot | Agent, Choice, G, D, Position, Proposal (each `Type 0`) | `actChoice : G -> Choice -> Choice`, after the program defines G and Choice |
| defined | Membership, Orbit, Obj, ChoiceRule, Aggregation, IsFixedPointObj, IsSelfConstituting, Config, LendingLedger, PrivateLoan | act, orbitProjection, Gov, amendPath, zkEquiv, borrow, repay, liquidate |
| primitive | Commitment, ZKProof | commit, commitUnit, commitAdd, verifyZK |

Name calls. `actChoice` names the PLAN slot "G-action on Choice".
`commitUnit` and `commitAdd` name the commitment monoid. `verifyZK` names
the ZKProof check. `Orbit` is not in the PLAN section 3 table; it comes
from the section 3 readings and is reserved so that the S4 port of
orbit-lang 9e28399 meets the same row as orbit-lang. `Proposal` names the
"Proposal fields" slot.

REFUSE_SIG rows (16). Each refuses in both modes. The orbit-lang rows
`sig-defined`, `sig-primitive`, `sig-primitive-op`, `sig-slot-universe`,
`sig-act-type` and `sig-act-early` are removed.

| Row | Form | Mutant |
|---|---|---|
| `sig-prim-commitment`, `sig-prim-zkproof` | `def Commitment : Type 0`, `def ZKProof : Type 0` (primitive types) | K6, K12 |
| `sig-prim-commit`, `sig-prim-commitunit`, `sig-prim-commitadd`, `sig-prim-verifyzk` | `commit`, `commitUnit`, `commitAdd`, `verifyZK` (primitive operations) | K6, K13 |
| `sig-def-membership` | `def Membership : Type 0` (the kernel HIT of S6) | K6, K11 |
| `sig-def-orbit` | `def Orbit : Type 0` (the orbit quotient of S4) | K6, K11, K14 |
| `sig-def-prelude-type`, `sig-def-prelude-op` | `Config`, `Gov` (DAO prelude of S7) | K6, K11 |
| `sig-def-ledger` | `LendingLedger` (the ledger of S7) | K6, K11 |
| `sig-def-zk`, `sig-def-step` | `zkEquiv`, `borrow` (the ZK ledger and a lending step of S8) | K6, K11 |
| `sig-slot-universe` | `def Agent : Type 1 := Type 0` (a slot at the wrong universe) | K7, K8 |
| `sig-actchoice-type` | `actChoice : Choice -> Choice` (the wrong slot type) | K7, K9 |
| `sig-actchoice-early` | `actChoice` before the program defines G and Choice | K7, K10 |

The defined rows are one per class (7 classes), not one per name. A
mutant that removes one table row of a defined name with no row (for
example Obj or repay) is not killed by a row; K11 kills the class.

Prop rows (4, O20: no proof irrelevance). The S2 rows `prop-type` and
`sprop-type` use Nat, thus they are kit rows; the S2 rows `prop-def` and
`sprop-def` are guest rows. S3 adds the other mode of each.

| Row | Mode | Form | Code | Mutant |
|---|---|---|---|---|
| `prop-type-guest` | guest | `def P : Prop := Unit` | NAME_UNKNOWN | K15 |
| `sprop-type-guest` | guest | `def P : SProp := Unit` | NAME_UNKNOWN | K16 |
| `prop-def-kit` | kit | `def Prop : Type 1 := Type 0` | REFUSE_NAME | K17 |
| `sprop-def-kit` | kit | `def SProp : Type 1 := Type 0` | REFUSE_NAME | K18 |

Mutants, one at a time (runner `lend-lang-work/S3-mutants.sh`, log
`lend-lang-work/S3-mutants.log`). The runner is a copy of
`orbit-lang-work/S3-mutants.sh` (K1 to K7, K2b, K4b; `row()`, `mut()`
and `admit()` verbatim) with K6 and K7 on the lend-lang rows, plus K8 to
K18 and a kit-mode helper `kadmit()`. `admit()` runs a guest row and
`kadmit()` runs a kit row: exit 0 with `ok` is an admission (KILLED), any
other result is a GAP. Each mutant: cp save, awk edit, mtime 2 s back,
`make -B build`, a stale `build/langc` refused, rows, cp restore. After
the loop, src/front/check.c and src/main.c are equal to every save (cmp)
and have the same shasum as before the loop, and the final `make -B
build` exits 0.

| Mutant | Site | Edit | Killer | Verdict |
|---|---|---|---|---|
| K1 INTERNAL builtin admitted | src/front/check.c:1890 | `kit_head`: `return 0 * (h->builtin->vis == VIS_INTERNAL);` | 15 rows `kit-type-*`, `kit-op-nat`, `kit-map-op`, `base-*`: want REFUSE_KIT, got `ok` | KILLED |
| K2 ORIGIN_KIT family admitted | src/front/check.c:1899 | `kit_head`: `return 0 * (fam != NO_FAMILY) && ...` | `kit-env`: want REFUSE_KIT, got `ok` | KILLED |
| K2b K1 + K2 | :1890, :1899 | both edits | `kit-pay`: want REFUSE_KIT, got `ok` | KILLED |
| K3 K1 + literal admitted | :1890, :1916 | K1 and `literal_ok`: `return 0 * guest(c) ? ...` | `kit-literal-nat`, `kit-literal-word`: want REFUSE_KIT, got `ok` | KILLED |
| K4 state and event admitted | src/front/check.c:2893 | `check_decl`: `if (0 * (d->is_event \|\| d->is_state) ...` | `kit-state`, `kit-event`: want REFUSE_KIT, got `ok` | KILLED |
| K4b K2 + K4 | :1899, :2893 | both edits | `kit-entry`: want REFUSE_KIT, got `ok` | KILLED |
| K5 kit mode always on | src/main.c:212 | `opt.mode = 1 ? CHECK_KIT : CHECK_GUEST;` | all 22 REFUSE_KIT rows: want REFUSE_KIT, got `ok` | KILLED |
| K6 reserved signature name admitted | src/front/check.c:2809 | `check_sig`: `return 1 ? 1 : FAIL(c, "REFUSE_SIG", "%s is a ...` | the 13 rows `sig-def-*`, `sig-prim-*`: want REFUSE_SIG, got `ok` | KILLED |
| K7 slot type not checked | src/front/check.c:2804 | `check_sig`: `return 1 \|\| slot_ok(...)` | `sig-slot-universe`, `sig-actchoice-type`, `sig-actchoice-early`: want REFUSE_SIG, got `ok` | KILLED |
| K8 type slot: Type 0 test admits | src/front/check.c:2786 | `slot_ok` SIG_TYPE: `return 1 \|\| conv_values(...)` | `sig-slot-universe`: want REFUSE_SIG, got `ok` | KILLED |
| K9 actChoice: slot type test admits | src/front/check.c:2793 | `slot_ok`: `return t != NULL;` (no conv with SIG_ACT) | `sig-actchoice-type`: want REFUSE_SIG, got `ok` | KILLED |
| K10 actChoice: order test admits | src/front/check.c:2791 | `slot_ok`: the G/Choice prerequisite returns 1 | `sig-actchoice-early`: want REFUSE_SIG, got `ok` | KILLED |
| K11 defined name admitted | src/front/check.c:2805 | `check_sig`: `case SIG_DEFINED: return 1;` | the 7 rows `sig-def-*`: want REFUSE_SIG, got `ok` | KILLED |
| K12 primitive type admitted | before src/front/check.c:2809 | `check_sig`: return 1 for SIG_PRIMITIVE and SIG_TYPE | `sig-prim-commitment`, `sig-prim-zkproof`: want REFUSE_SIG, got `ok` | KILLED |
| K13 primitive operation admitted | before src/front/check.c:2809 | `check_sig`: return 1 for SIG_PRIMITIVE and SIG_OP | `sig-prim-commit`, `sig-prim-commitunit`, `sig-prim-commitadd`, `sig-prim-verifyzk`: want REFUSE_SIG, got `ok` | KILLED |
| K14 Orbit row dropped | src/front/check.c:2740 | SIGNATURE: the row `{"Orbit", SIG_TYPE, SIG_DEFINED}` removed | `sig-def-orbit`: want REFUSE_SIG, got `ok` | KILLED |
| K15 guest Prop resolves as Unit | src/front/check.c:1851 | head lookup: the builtin Unit for the name `Prop` | `prop-type-guest`: want NAME_UNKNOWN, got TYPE_MISMATCH | KILLED |
| K16 guest SProp resolves as Unit | src/front/check.c:1851 | head lookup: the builtin Unit for the name `SProp` | `sprop-type-guest`: want NAME_UNKNOWN, got TYPE_MISMATCH | KILLED |
| K17 Prop not reserved | src/front/check.c:184 | RESERVED: `"Prop"` removed | `prop-def-kit` under `--kit`: want REFUSE_NAME, got `ok` | KILLED |
| K18 SProp not reserved | src/front/check.c:184 | RESERVED: `"SProp"` removed | `sprop-def-kit` under `--kit`: want REFUSE_NAME, got `ok` | KILLED |

Score: 20 of 20 killed, 79 admissions, 0 GAP. The table shapes give two
primitive kinds (SIG_TYPE and SIG_OP rows of SIG_PRIMITIVE), thus K12 and
K13; the defined kind is one switch case, thus the single K11, plus K14
for the Orbit row that the S4 port needs. K15 and K16 are not admissions:
`Type 0` is syntax, not a name, so no single-site name edit makes `Prop` a
universe. They use the S2 M6 shape (the name resolves as Unit), and the
guest row changes from NAME_UNKNOWN to TYPE_MISMATCH. K17 and K18 remove
the RESERVED entries that S2 M1 removed (M1 killed the guest rows
`prop-def` and `sprop-def`); the runner checks only the new kit rows. The
16 REFUSE_SIG rows also give REFUSE_SIG under `--kit` (checked by hand,
16 of 16).

Builder deaths. In S3 session 1 the wf-builder stage died before any file
change, and the opus fallback died too. The signature swap is built by
hand.

| Session | Fable xhigh (429, usage limit) | opus xhigh fallback (`reasoning_extraction`) |
|---|---|---|
| 1 | req_011Cg8rYD9gsEoBrNUVtitST | req_011Cg8rcsqDwcS29CFnEXgZc |

## S4

Slice S4 (the orbit quotient and the dependent eliminator, PLAN section
8) is complete on commit 993ee13 (the USER S4 commit on ee9b215). The
code ports orbit-lang 9e28399. The kernel declares Orbit and
orbitProj directly after the program `def actChoice`, the G-action on
Choice, and `elim P c1 .. cn x` eliminates Sum A B and Unit into a
motive. The port is one patch, `git apply --3way`, with 3 conflict files
(SPEC.md, test/check/expect.txt, docs/VALIDATION.md) and 0 code conflicts
(PLAN section 8 "Builder call (S4, 2026-10-17)"). lend-lang keeps its
deltas: the USER review fix of 2449b67 (`conv_system`), the S3 review fix
(`sig_member_free`), the Orbit kit row, and the committed orbitProj
signature reservation and its regressions in test/fronttool.c. The gate
`make -B check` is GREEN, exit 0 (logs `lend-lang-work/S4-gate.log` and
`lend-lang-work/S4-gate-final.log`).

| Count | S3 (ee9b215) | S4 | orbit-lang S4 | Reason |
|---|---|---|---|---|
| examples | 25 | 26 | 26 | `orbit` (guest): isSet Choice by elim, the fold of Orbit into Choice |
| parse refusals | 19 | 19 | 19 | no change |
| check refusals | 133 | 146 | 132 | the 13 orbit-lang S4 rows |
| kit-only refusals | 22 | 22 | 22 | no change |
| evals | 62 | 65 | 65 | `orbit agreeAll`, `orbit agreeSplit`, `orbit agreeFlipped` |
| lower refusals | 14 | 15 | 15 | `orbit-word` |
| build checks | 31 | 32 | 32 | `orbit-word` |

Check refusals 146 against 132: orbit-lang S4 is orbit-lang S3 (119) plus
13; lend-lang S4 is lend-lang S3 (133) plus the same 13. The other counts
do not change and are equal to S3 and to orbit-lang S4: runs 5, run
refusals 10, asmtool 8 of 8, builds 3, dispatch 20, evm steps 43,
precompile steps 18, mock 13 rows, diff 15 rows, emit 24, and the review,
domain, path, fuel, output, lower and hashing regressions.

The 13 check rows (test/check/expect.txt), all guest rows.

| Row | Form | Code | Mutant |
|---|---|---|---|
| `orbit-guest-def`, `orbitproj-guest-def` | a program `def Orbit` or `def orbitProj` after actChoice | REFUSE_SIG | G0, G1 |
| `orbit-no-act` | Orbit with no `def actChoice` | NAME_UNKNOWN | O1 |
| `orbit-fold-no-set` | a fold of Orbit with no isSet case | TYPE_MISMATCH | F1 |
| `orbit-fake-set` | a false isSet case | PATH_END | P1b |
| `orbit-boundary` | a quotEq path with a wrong witness | PATH_END | P1 |
| `elim-scrutinee` | elim on a scrutinee that is not Sum or Unit | TYPE_MISMATCH | E1 |
| `elim-case-count` | elim with a wrong case count | TYPE_MISMATCH | E2 |
| `elim-motive-dom` | elim with a wrong motive domain | TYPE_MISMATCH | E3 |
| `elim-case-type` | elim with a wrong case type | TYPE_MISMATCH | E4 |
| `elim-path-k` | elim on a Path (no K) | TYPE_MISMATCH | E1b |
| `elim-motive-prop` | a motive into Prop | NAME_UNKNOWN | N1b |
| `unit-no-eta` | Unit eta | PATH_END | P1 |

Adaptation (by hand, PLAN section 8): (1) src/front/front.c,
`orbit_after_act_choice`: the trigger is the program `def actChoice`
(orbit-lang: `def act`), and ORBIT_R is Quot over Agent -> Choice with R
c d = Sigma (g : G) (Path (Agent -> Choice) (fun (a : Agent) =>
actChoice g (d a)) c). (2) src/front/check.c: the kernel_sig comment and
message say "after actChoice". Review before 993ee13 adds `orbitProj` as
SIG_OP/SIG_DEFINED to SIGNATURE, bringing S4 to 19 defined names from the
18 of S3. The other rows do not change. `act` stays reserved, thus the
examples use the helper `actObj`. (3) The
examples and rows use Agent, G, D, `def actChoice` and `def actObj` for
the orbit-lang Member, Group, Outcome and `def act`.

Design note. PLAN section 3 gives Orbit M := Obj M / act M, over the
current members, with member permutation (O8). S4 ports the orbit-lang
form: Orbit over all of Agent -> Choice, choices only. The M index and
the member permutation wait for the slice that defines Obj and act (S7).

The fronttool row. test/fronttool.c adds `ok &= kit("state State :=
Orbit", "REFUSE_SIG");` (S4 session 2). sig_member_free reads the
SIGNATURE rows, and kernel_sig refuses `def Orbit` before the table, thus
`sig-def-orbit` alone does not kill a removed Orbit row. The kit row
kills it (K14r). The committed review also tests `event orbitProj`,
`state State := orbitProj`, and `state State := makeState (orbitProj :
Nat)` as REFUSE_SIG before actChoice is defined. Positive cases keep
`event Opened (orbitProj : Nat)` and trusted `family Local := orbitProj`
valid.

Mutants, one at a time (runner `lend-lang-work/S4-mutants.sh`, log
`lend-lang-work/S4-mutants.log`, run 1 log
`lend-lang-work/S4-mutants.run1.log`). This original loop predates the
orbitProj SIGNATURE row in 993ee13: removing only that row reproduces
the logged check.c SHA-1, `4a9a07a5e86f77b78de42fd751cc8d808b370c23`.
The table and restoration claims below describe that earlier snapshot.
The runner is a copy of
`orbit-lang-work/S4-mutants.sh` (G0, G1, O1, E1 to E4, F1, P1, N1, L1,
P1b, E1b, N1b, probe L1r) plus A1, S1 and K14r. The deltas: the O1
regexes anchor `"actChoice") != 0` and the lend-lang ORBIT_R; the third
G1 edit anchors at a two-space indent, so that the sig_member_free FAIL
(six spaces) does not change and only the 2 kernel_sig lines change, as
in orbit-lang; `mut()` also needs a `build/fronttool` newer than the
edit; the pristine pass also checks `examples/orbit.lang` (ok) and
`build/fronttool` (pass). Each mutant: cp save, edit, `make -B build`,
rows, cp restore.

| Mutant | Site | Edit | Killer | Verdict |
|---|---|---|---|---|
| G0 kernel_sig admits alone | src/front/check.c:2915 | `if (1 \|\| d->origin != ORIGIN_PROGRAM \|\| ...)` | `orbit-guest-def`, `orbitproj-guest-def`: want REFUSE_SIG, got REFUSE_NAME (rule R4) | KILLED (diagnostic) |
| G1 G0 + name_free + the defined-name test | :2350, :2908, :2915, :2917 | each `return FAIL(...)` gives 1, and G0 | `orbit-guest-def`, `orbitproj-guest-def`: want REFUSE_SIG, got `ok` | KILLED |
| O1 Orbit with no act | src/front/front.c:17, :30 | ORBIT_R: `Path (Agent -> Choice) d c`; the trigger is `"G"` | `orbit-no-act`: want NAME_UNKNOWN, got `ok` | KILLED |
| E1 elim scrutinee test admits | src/front/check.c:855 | `if (0 * !val_is(st, OP_SUM) && ...)` | `elim-scrutinee`: want TYPE_MISMATCH, got `ok`; GAP `elim-path-k` (TYPE_INFER) | KILLED |
| E2 case count from the call | :857 | `want = k->argc - 2u;` | `elim-case-count`: want TYPE_MISMATCH, got `ok` | KILLED |
| E3 scrutinee type for the motive domain | :854 | `st` is the try_infer type of the last argument, else `pt->dom` | `elim-motive-dom`: want TYPE_MISMATCH, got `ok` | KILLED |
| E4 inl case inferred | :865 | `i == 0u ? infer(c, k->args[1], &pt) : check(...)` | `elim-case-type`: want TYPE_MISMATCH, got `ok` | KILLED |
| F1 a fold fills a missing last case with the first | :1134, :1144, :1159 | `j + 1u >= k->argc ? cores[0] : ...`, `k->used = k->argc;`, `ctor_count <= k->argc` | `orbit-fold-no-set`: want TYPE_MISMATCH, got `ok` | KILLED |
| P1 end_fail passes a PATH_END | :469 | `return 1 ? 1 : FAIL(c, "PATH_END", ...)` | `orbit-boundary`, `unit-no-eta`: want PATH_END, got `ok`; GAP `orbit-fake-set` (PATH_UIP) | KILLED |
| N1 the domain gains a sort Prop | domain/domain.lang | `def Prop : Type 1 := Type 0` | GAP `elim-motive-prop` (REFUSE_NAME: RESERVED has Prop, rule R4) | SURVIVED |
| L1 elim on a formal hcomp reduces as elim on its base | src/front/eval.c:731 | reduce_elim on the hcomp base for the stuck elim (O22) | GAP `orbit-word` (REFUSE_LOWER: K4c does not lower elim at a word position) | SURVIVED |
| P1b P1 + end_fail passes a PATH_UIP | src/front/check.c:468, :469 | both `return 1 ? 1 : FAIL(...)` | `orbit-fake-set`: want PATH_END, got `ok` | KILLED |
| E1b E1 + a K case type | :801, :855 | `elim_case`: a PathP scrutinee gives the one case type P refl; and E1 | `elim-path-k`: want TYPE_MISMATCH, got `ok` | KILLED |
| N1b N1 + RESERVED without Prop | domain/domain.lang, src/front/check.c:186 | N1, and RESERVED is `{"Bool", "Text", "SProp"}` | `elim-motive-prop`: want NAME_UNKNOWN, got `ok` | KILLED |
| A1 the Orbit trigger is act | src/front/front.c:30 | `"act"` for `"actChoice"` | `examples/orbit.lang`: want `ok`, got NAME_UNKNOWN (unknown name Orbit) | KILLED |
| S1 check_ctor does not test the signature | src/front/check.c:2525 | `if (!name_free(c, ctor->name))` | `build/fronttool`: want REFUSE_SIG, got `ok`: `state State := Commitment` | KILLED |
| K14r the Orbit row removed from SIGNATURE | :2824 | the row `{"Orbit", SIG_TYPE, SIG_DEFINED},` removed | `build/fronttool`: want REFUSE_SIG, got `ok`: `state State := Orbit` | KILLED |
| L1r probe (not counted) | src/front/eval.c:731 | `return internal(m, "L1r");` | GAP `orbit-word` (the same REFUSE_LOWER) | SURVIVED |

Score: 15 of 17 killed. N1 and L1 survive, as in orbit-lang (12 of 14
there). N1: RESERVED has Prop, thus `def Prop` meets REFUSE_NAME (rule
R4) before the motive test; N1b kills the pair. L1: `orbit-word` refuses
with REFUSE_LOWER (K4c does not lower elim at a word position,
src/lower.c:645) before the reduce_elim hcomp line. The probe L1r makes
that line INTERNAL and gives the same REFUSE_LOWER, thus the line is not
reached and L1 is equivalent for `orbit-word`.

E2 and E3 retarget. The orbit-lang E2 and E3 anchors (`uint32_t want =
val_is(st, OP_SUM) ? 2u : 1u;` and `|| !conv_values(c->m, c->level,
pt->dom, st))`) are not in orbit-lang 9e28399: the orbit-lang mutant log
(2026-10-17 12:22) is older than the 9e28399 commit (12:47), and rule_elim
changed after that loop. Thus run 1 gave E2 and E3 INVALID (the edit
changed nothing). Run 2 anchors `want = val_is(st, OP_SUM) ? 2u : 1u;`
(E2) and `st = pt->dom;` (E3). For E3, elim_domain returns the motive
annotation, thus a fallback form would be equivalent; the edit uses the
inferred type of the scrutinee in place of the motive domain.

After the loop, src/front/check.c is equal to every check.c save,
src/front/front.c to the A1 save, src/front/eval.c to the L1r save and
domain/domain.lang to the N1b save (cmp). The shasum of the four files is
the same before and after the loop, and the final `make -B build` exits 0.

Staged documentation review (2026-10-18): the same runner, with only its
R and S directories changed to an isolated checkout, repeats the loop on
993ee13. It again kills 15 of 17 mutants; N1 and L1 survive, and the
separate L1r probe survives. There are no invalid mutants. The restored
check.c SHA-1 is `678de5305a644ed38e5ff00c0380b92da840804e`, and all four
source hashes match before and after. A full `make -B check` on this
source also passes. Review artifacts are under
`/private/tmp/lend-lang-review.23pBI4`: the gate capture is
`repo/.kanon-exec/run-mYXMXX` and the mutant capture is
`.kanon-exec/run-8MIE1y`.

Builder deaths. Each tier died before any file change. The port, the
mutants and the records are built by hand.

| Session | Fable xhigh (429, usage limit) | opus xhigh fallback (`reasoning_extraction`) |
|---|---|---|
| 1 (port) | req_011Cg9C2NqbCP9jH4Baoyh7o | req_011Cg9C5T7u3seuat8mYPr2E |
| 2 (records) | req_011Cg9HyC5ZJvfdiGSiWPMCQ | req_011Cg9JH3RfDR42Tgz8EYv17 |

## S5

S5 ports orbit-lang a12973a (Glue, ua, uaBeta, hcomp at Glue and U;
orbit-lang ports them from coin-lang 44c917f, 91d7091 and 34acff5) on
68ce871, the USER S4 records commit on 993ee13.

### Counts

| Gate line | lend-lang S4 | orbit-lang S5 | lend-lang S5 |
|---|---|---|---|
| examples | 26 | 29 | 29 |
| parse refusals | 19 | 19 | 19 |
| check refusals | 146 | 147 | 161 |
| kit-only refusals | 22 | 22 | 22 |
| evals | 65 | 75 | 75 |
| runs, run refusals | 5, 10 | 5, 10 | 5, 10 |
| asmtool checks | 8 | 8 | 8 |
| builds | 3 | 3 | 3 |
| lower refusals | 15 | 16 | 16 |
| build checks | 32 | 33 | 33 |
| dispatch | 20 | 20 | 20 |

Each lend-lang S5 delta equals the orbit-lang S4 to S5 delta: 3 examples
(equiv, glue, univ), 15 check refusals (16 new rows; hcomp-universe goes
out, because hcomp at Type 0 is now a Glue type), 10 evals, 1 lower
refusal (glue-word) and 1 build check. The other counts (evm steps 43
and the rest) equal S4. The lend-lang check count stays above the
orbit-lang count by the lend-lang signature and Prop rows of S3 and S4.

### Port

The patch is `git -C ~/Documents/orbit-lang diff 9e28399 a12973a -- .`
without docs/PLAN.md, README.md, design and docs/STATUS.md: 38 files, 83
hunks, +1527 -76. `git apply --3way` gives the same 5 conflict files in
a scratch clone and in the repo, and 0 other conflicts.

| File | Hunks | Resolution |
|---|---|---|
| src/front/eval.c | 2 | the lend-lang conv_system, conv_branches and mark_intervals (2449b67) stay; the orbit-lang S5 conv_system goes out; conv_loop keeps one call |
| test/pathtool.c | 3 | both sides stay: the lend-lang `rejected` helper and 5 system rows, the orbit-lang `print_refused`, `accepts`, `system_conversion` and rows |
| SPEC.md | 1 | the lend-lang name, F1 to F17 |
| examples/orbit.lang | 1 | the lend-lang `agreeFlipped` (actObj) stays; the orbit-lang S5 lines flipEquiv to gElim are added |
| docs/VALIDATION.md | 2 | the lend-lang counts and paragraphs stay; the S5 paragraph is adapted |

Adaptation: no added line uses act, Member, Group or Outcome, thus the
new files need no name change. The S5 changes to src/front/check.c do
not touch the SIGNATURE table, and the new domain/domain.lang
definitions pass the lend-lang signature check. The orbit-lang S5
conv_system compares branch values without the face. The lend-lang
rule (USER review fix of 2449b67) compares the covers, then each overlap
on its face. The orbit-lang system_conversion test (order, values,
coverage, refinement) passes on the lend-lang rule.

### Mutants

~/Documents/lend-lang-work/S5-mutants.{sh,log}. The script is the
orbit-lang S5 script (M1 to M9) with the lend-lang paths. mut() needs
build/langc and build/fronttool newer than the mutant, with `make -B`
per build. Two lend-lang mutants (C1, C2) attack conv_system, where the
lend-lang delta meets the S5 system_conversion test. Before the run,
`rg -c -F` found each anchor once in lend-lang, at the same line as in
orbit-lang a12973a, thus no anchor needs a retarget (the orbit-lang log
predates the a12973a commit, the S4 trap).

| Id | Target | Change | Result | Killer |
|---|---|---|---|---|
| M1 | check.c:2507 elab_inner | the Fin literal bound test never fires | KILLED | fin-range admitted |
| M2 | check.c:2411 finAll | a failed case takes case 0 | KILLED | fin-all-case, fin-case-stuck admitted |
| M3 | check.c:1528 Glue cover | the cover test never fires | SURVIVED | glue-cover refused with PATH_END (a second class, as in orbit-lang) |
| M4 | check.c:1619 glue_tube | the binder test never fires | KILLED | glue-binder admitted |
| M5 | check.c:1652 glue_branch | the inner failure is kept | KILLED | glue-equiv gives TYPE_MISMATCH, not GLUE_EQUIV |
| M6 | check.c:1720 glue_boundary | the boundary test never fires | KILLED | glue-boundary admitted |
| M7 | check.c:1783 rule_unglue | unglue of another type is the term | KILLED | unglue-type admitted |
| M8 | check.c:1580 rule_hcomp | n != 0 becomes n > 1 | KILLED | hcomp-higher-universe admitted |
| M9 | eval.c:2607 glue_at | the fallback is INTERNAL | KILLED | circle loopAt0 and loop2At0 give INTERNAL |
| C1 | eval.c:3111 conv_system | the cover equality test never fires | KILLED | pathtool system conversion (coverage) |
| C2 | eval.c:3072 conv_branches | each overlap compares equal | KILLED | pathtool `systems with distinct live branches remain unequal` (PATH_UIP), system conversion |

The loop kills 10 of 11 mutants with 7 admissions. M3 survives for the
orbit-lang reason. Each mutated file is restored. The shasums of
src/front/check.c, src/front/front.c, src/front/eval.c,
domain/domain.lang and test/pathtool.c are equal before and after the
loop. No mutant is INVALID.

### Builders

| Stage | Tier | Result |
|---|---|---|
| apply, gate, mutants | wf-builder Fable xhigh | died, 429 (req_011Cg9QZ7NBC7zTpR19EajAp), before a file change |
| apply, gate, mutants | wf-builder opus xhigh | died, `reasoning_extraction` (req_011Cg9Qc1RLKE6USFQBBTFi5), before a file change |
| apply, gate, mutants, records | main loop | by hand |

## S6

S6 ports orbit-lang 8dd82a3 (indexed families, `and` blocks, block
folds, HIT_INDEX, BLOCK_ORDER) on 4484ce6, the USER S5 commit on
68ce871. Two new parts follow: O21 (`elim` over a uniform domain family
of points and 1-dimensional paths) and Membership (a kernel HIT after
`def Agent`). PLAN section 8, "Builder call (S6, 2026-10-18)", has the
design.

### Counts

| Gate line | lend-lang S5 | after the port (session 1) | lend-lang S6 |
|---|---|---|---|
| examples | 29 | 31 | 32 |
| parse refusals | 19 | 19 | 19 |
| check refusals | 161 | 162 | 164 |
| kit-only refusals | 22 | 22 | 22 |
| evals | 75 | 80 | 85 |
| runs, run refusals | 5, 10 | 5, 10 | 5, 10 |
| asmtool checks | 8 | 8 | 8 |
| builds | 3 | 3 | 3 |
| lower refusals | 16 | 16 | 16 |
| build checks | 33 | 33 | 33 |
| dispatch | 20 | 20 | 20 |

The port adds 2 examples (hiit, qiit), 1 check refusal and 5 evals.
Session 3 adds 1 example (membership), 2 check refusals
(membership-premise, membership-missing-case) and 5 evals (copySet,
copyExitEnd, windAdmit, windExit, windAdmitEnd). The evm steps (43) and
the other counts equal S5.

### Port

The patch is `git -C ~/Documents/orbit-lang diff a12973a 8dd82a3 -- .`
without docs/PLAN.md, README.md, design and docs/STATUS.md: 14 files, 64
hunks, +635 -76. `git apply --3way` gives 1 conflict file,
test/fronttool.c (1 hunk: the lend-lang `kit` helper and the orbit-lang
`normal_roundtrip_in` both stay), and 0 code conflicts in src. The
orbit-lang S6 B4 rows are not in a commit, thus they are not ported.

### O21 and Membership

- O21 (session 2): rule_elim accepts a third scrutinee class, a uniform
  domain family of points and 1-dimensional paths. check.c
  `elim_family_case` gives the type of each case: a function of the
  fields, with the hypothesis `P x` after each recursive field, to
  `P (c fields)` or to `PathP (fun i => P (p fields i)) E0 E1`. eval.c
  `elim_ctor` applies the case and, for a path constructor, path_apply
  at r with the ends E0 and E1.
- Fix (session 3): the empty Env is NULL. For a family with no
  parameters (Membership, S1), the field loop stopped at the first field
  and `elim_family_case` returned NULL with no diagnostic (`langc:
  (null): :`). Now the flag `ok` records a failed env_push (the OOM
  diagnostic).
- Membership (session 2): front.c puts kernel/membership.lang
  (ORIGIN_DOMAIN) directly after the program `def Agent`. Each field
  name has the constructor name as prefix.
- Deviation 1 (session 2): the point constructor is `memberSet`, not
  `base`. S1 owns `base` (REFUSE_NAME, rule R4).
- Deviation 2 (session 3): there is no elim row into a 2-valued type.
  admit and exit join each member set to each other member set, thus a
  consistent map from Membership to Sum Unit Unit is constant and its
  path cases are refl. A refl case cannot show a difference between
  path_apply at r and path_apply at i0. Thus the eval rows use copy
  (into Membership) and wind (into S1).

### Mutants

~/Documents/lend-lang-work/S6-mutants.{sh,log}. mut() and the row
helpers are the S5 helpers; ex() and front() are the orbit-lang S6
helpers. mut() needs build/langc and build/fronttool newer than the
mutant, with `make -B` per build. HI1, HI2, BO and BOb are the
orbit-lang S6 mutants. Before the run, `rg -c -F` found each anchor
once.

| Id | Target | Change | Result | Killer |
|---|---|---|---|---|
| E1 | check.c:1001 rule_elim | no scrutinee is a domain family (`fam = 0`) | KILLED | membership.lang: TYPE_MISMATCH at loopAt |
| E2 | check.c:1005 rule_elim | the case count test allows one case less | KILLED | membership-missing-case: segmentation fault (exit 139); rule_elim reads the scrutinee past the arguments |
| E3 | check.c:940 elim_family_case | the PathP ends are swapped | KILLED | membership.lang: PATH_UIP at loopAt |
| E4 | eval.c:783 elim_ctor | path_apply at the i0 end, not at r | KILLED | eval windAdmit gives `fun i => base` |
| E5 | front.c:72 orbit_after_act_choice | the splice looks for `AgentX` | KILLED | membership.lang: NAME_UNKNOWN Membership at admitOne |
| E6 | front.c:32 MEMBERSHIP_STEP | the P field ends at the `to` value | KILLED | membership-premise admitted |
| E6b | check.c end_fail | PATH_END never fires | KILLED | membership-premise admitted (the row has no second refusal class) |
| E7 | check.c:925 elim_family_case | `ok` also needs p > 0 (the stop at the first field for a family with no parameters) | KILLED | membership.lang: OOM at loopAt |
| HI1 | check.c ctor_result | a constructor of an indexed family with no result passes | KILLED | fronttool domain: wanted HIT_INDEX, got ok |
| HI2 | check.c ctor_result | the result head test never fires | SURVIVED | the orbit-lang killer `hiit-wrong-head` is an orbit-lang B4 row, not in a commit, not ported |
| BO | check.c header_order | a header that names a later sort passes | KILLED | fronttool domain: wanted BLOCK_ORDER, got REFUSE_REC |
| BOb | check.c header_order and unknown_name | BO and the R3 later-name test both pass | KILLED | fronttool domain: wanted BLOCK_ORDER, got NAME_UNKNOWN |

The loop kills 11 of 12 mutants with 2 admissions (E6, E6b). E2 is a
crash kill: the gate row fails, but with a segmentation fault, not with
a refusal. No row meets a second refusal class (E6b). Each mutated file
is restored. The shasums of src/front/check.c, src/front/front.c and
src/front/eval.c are equal before and after the loop. No mutant is
INVALID.

### Builders

| Stage | Tier | Result |
|---|---|---|
| port, O21, Membership, rows | wf-builder Fable xhigh | died in session 1, 429, before a file change |
| port, O21, Membership, rows | wf-builder opus xhigh | died in session 1, `reasoning_extraction`, before a file change |
| sessions 1 to 4 | main loop | by hand; the session 4 records by a wf-closer from fixed text |

## S7

S7 adds the DAO prelude and the lending ledger on 1d7369a, the USER S6
commit. orbit-lang S7 (96c74e1) has no code, thus lend-lang builds its
own prelude and uses the orbit-lang B2 text as the model. PLAN section
8, "Builder call (S7, 2026-10-18)", has the design, and "Result (S7,
...)" has the deviations.

### Counts

| Gate line | lend-lang S6 | Part 0 (session 1) | Part 1 (session 2) | Part 2 (session 5) | lend-lang S7 |
|---|---|---|---|---|---|
| examples | 32 | 32 | 33 | 33 | 34 |
| parse refusals | 19 | 19 | 19 | 19 | 19 |
| check refusals | 164 | 166 | 167 | 169 | 170 |
| kit-only refusals | 22 | 22 | 22 | 22 | 22 |
| evals | 85 | 85 | 85 | 85 | 88 |
| runs, run refusals | 5, 10 | 5, 10 | 5, 10 | 5, 10 | 5, 10 |
| asmtool checks | 8 | 8 | 8 | 8 | 8 |
| builds | 3 | 3 | 3 | 3 | 3 |
| lower refusals | 16 | 16 | 16 | 16 | 16 |
| build checks | 33 | 33 | 33 | 33 | 33 |
| dispatch | 20 | 20 | 20 | 20 | 20 |

Part 0 adds 2 check refusals (block-fold-no-squash, block-fold-target)
and a fronttool HIT_INDEX row. Part 1 adds 1 example (dao) and 1 check
refusal (dao-constitution-refl). Part 2 adds 2 check refusals
(ledger-exit-keeps, dao-sc-prop). Part 3 adds 1 example (ledger), 1
check refusal (dao-wrong-unit) and 3 evals (exitAt, exitOther, backAt).
The evm steps (43) and the other counts equal S6. The S7 gate runner is
~/Documents/lend-lang-work/S7-p3-gate.sh.

### Parts

- Part 0 (session 1): the patch is
  `git -C ~/Documents/orbit-lang diff 8dd82a3 8b11846 -- test/` (4
  files, +39). The end context of test/check/expect.txt is different,
  thus the 2 new lines are appended. The fronttool HIT_INDEX row kills
  HI2, the S6 survivor (~/Documents/lend-lang-work/S7-p0-HI2.log).
- Part 1 (session 2): front.c `dao_source` (kernel/dao.lang,
  ORIGIN_DOMAIN) has Obj, act, orbitProjection, ChoiceRule,
  Constitution, Aggregation, Gov, IsFixedPointObj and
  IsSelfConstituting. `splice_after_all` puts it after the last of the
  program slots in DAO_AFTER (Agent, Choice, G, D, actChoice,
  orbitProj).
- Part 2 (sessions 3 to 5): front.c `ledger_source` (kernel/ledger.lang)
  has Holding, Inv, Ledger, the isProp lemmas, invPathP, ledgerPath,
  holdSwap, sel, selSel, fwdInv, bwdInv, stepEquiv, admitEquiv,
  exitEquiv, LendingLedger and Config. It goes after the last of
  LEDGER_AFTER (Position, IsSelfConstituting). eval.c conv_values
  compares the two pointers before conv_loop (deviation 5).
- Part 3 (sessions 6 and 7): examples/ledger.lang transports l1 (at
  onlyOne) along exitOne with
  `transp (fun i => LendingLedger (exitOne @ i)) i0 l1`.
  examples/dao.lang has the laws amendGov, anon0, govUnit and scPath.

| Row | Code | PLAN gate row |
|---|---|---|
| dao-constitution-refl | PATH_END | a non-invariant constitution |
| ledger-exit-keeps | PATH_END | an exit that does not forfeit |
| dao-sc-prop | TYPE_MISMATCH | the ∃ of IsSelfConstituting as a Prop (`trunc1 T sc0`) |
| dao-wrong-unit | PATH_END | a wrong unit; the control (F1 gives inl unit) is ok |

| Eval row | Result | Reading |
|---|---|---|
| ledger exitAt | `inr (inl (pair unit unit))` | Forfeited p at the agent of the exit |
| ledger exitOther | `inr (inr unit)` | Empty at another agent: no change |
| ledger backAt | `inl (pair unit unit)` | Active p again after the admit |

### amendPath

amendPath is not in S7 (PLAN deviation 7, open item O27). The record is
~/Documents/lend-lang-work/S7-p3-record.md, and the probes are in
~/Documents/lend-lang-work/S7-p3-scratch/.

- The fill `transp (fun j => LendingLedger (exitOne @ (i /\ j))) (~i) l1`
  passes rule_transp, but reaches the arena limit in 0.7 s. Each
  endpoint alone checks.
- oomv-e (r12): `refl` at the exitEquiv value, at its Equiv type,
  reaches the arena limit. `refl` at its forward map (r13) checks. Thus
  the limit is in the conv of the isEquiv part.
- oomv-f (a1): `refl : Path Config cA cA` (closed, M constant) reaches
  the arena limit. Each part alone checks.
- The kernel has no regularity (r6): transp along a constant line at i0
  is not its argument (PATH_END).

### Mutants

~/Documents/lend-lang-work/S7-mutants.{sh,log}. fmut() is a fixed-string
replace (awk -v), with `make -B` per build. Before the run, each anchor
count is 1, and the pristine build gives each row and eval as expected.

| Id | Target | Change | Result | Killer |
|---|---|---|---|---|
| P1 | front.c:69 DAO_AFTER | the splice looks for `orbitProjX` | KILLED | dao.lang: NAME_UNKNOWN at F0 (unknown name ChoiceRule) |
| P2 | front.c:227 LEDGER_AFTER | the splice looks for `IsSelfConstitutingX` | KILLED | ledger.lang: NAME_UNKNOWN at hold1 (unknown name Holding) |
| P3 | front.c:90 ledger_source Inv | Forfeited p at a member is Unit, not `Path 2 1 0` | KILLED | the kernel text: TYPE_MISMATCH at isPropInv (ledger.lang and dao.lang) |
| P4 | front.c:132 ledger_source sel | the swap branch keeps h (no holdSwap) | KILLED | the kernel text: TYPE_MISMATCH at selSel; the eval row ledger exitAt differs |
| P5b | check.c:486 end_fail | PATH_END never fires | KILLED | dao-wrong-unit, ledger-exit-keeps and dao-constitution-refl admitted |
| P6 | eval.c:3322 conv_values | no `a == b` test | KILLED | dao.lang: OOM at cfg0 (the arena limit, 1 GB) |

The loop kills 6 of 6 mutants with 1 admission mutant (P5b). P3 and P4
are killed by the check of the kernel text, not by a guest refusal row:
the prelude is checked with each program, thus each program that has
the prelude fails. P5b is the "b" mutant: no S7 PATH_END row meets a
second refusal class. The P6 arena limit (1 GB) is in the 4 GB rule.
Each mutated file is restored. The shasums and
`git diff --stat src/` are equal before and after the loop. No mutant
is INVALID.

### Builders

| Stage | Tier | Result |
|---|---|---|
| Part 1 probe | wf-builder Fable xhigh | died in session 2, 429, before a file change |
| Part 2 probe | wf-builder Fable xhigh | died in session 3, 429, before a file change |
| Part 3 | wf-builder Fable xhigh, then opus xhigh | died in session 6 (429; `reasoning_extraction`), before a file change |
| records, stage | wf-builder Fable xhigh, then opus xhigh | died in session 8 (429; `reasoning_extraction`), before a file change |
| sessions 1 to 9 | main loop | by hand; the USER applied the session 4 src changes (S7-p2-apply.sh) |

## S8

S8 ports orbit-lang B1 676c7a8 (dependent elim for domain families) and
B2 eafa1df (the outcomeSet and menu slots, with lend names) on 7d6a401,
the USER S7 commit. It adds law 2 uniqueness and the O27 (b) fast path,
and with it the guest term amendExit in examples/dao.lang. amendPath
stays a reserved SIG_DEFINED name. PLAN section 8, "Builder call (S8,
2026-10-18)", has the design and the result paragraphs.

### Counts

| Gate line | lend-lang S7 | Part 1 (session 5) | Part 2 (session 7) | Part 3 (session 11) | lend-lang S8 |
|---|---|---|---|---|---|
| examples | 34 | 35 | 35 | 35 | 35 |
| parse refusals | 19 | 19 | 19 | 19 | 19 |
| check refusals | 170 | 175 | 179 | 180 | 181 |
| kit-only refusals | 22 | 22 | 22 | 22 | 22 |
| evals | 88 | 96 | 98 | 101 | 101 |
| runs, run refusals | 5, 10 | 5, 10 | 5, 10 | 5, 10 | 5, 10 |
| asmtool checks | 8 | 8 | 8 | 8 | 8 |
| builds | 3 | 3 | 3 | 3 | 3 |
| lower refusals | 16 | 16 | 16 | 16 | 16 |
| build checks | 33 | 33 | 33 | 33 | 33 |
| dispatch | 20 | 20 | 20 | 20 | 20 |

Part 1 adds 1 example (elim), 5 check refusals and 8 evals. Part 2 adds
4 check refusals and 2 evals. Part 3 adds 1 check refusal (amend-end)
and 3 evals. Session 12 adds 1 check refusal (menu-early). The evm
steps (43) and the other counts equal S7. The gate logs are
~/Documents/lend-lang-work/S8-p1-port-gate.log, S8-p2-port-gate.log,
S8-s11-port-gate.log and S8-s12-port-gate.log.

### Parts

- Part 0 (session 2): the O27 (b) probe, in a copy only. The conv head
  alone (try 1) clears lFill and dh. The conv head with the
  head-preserving quote_value (try 2) clears r12, a1, lFill, a2h and dh
  and the four controls. The copy gate counts equal S7. Record:
  ~/Documents/lend-lang-work/S8-o27b-probe.md.
- Part 1 (sessions 3 to 5): the B1 port (S8-p1.patch, 1175 lines).
  check.c takes the B1 elim_on_family, and the S6 uniform-family branch
  drops out. Try 1 failed one lend S6 test (test/fronttool.c, the
  overapplied Unit eliminator that ends in swOff). USER ruling (a): if
  the motive has no binder annotation and the scrutinee scan gives a
  family, rule_elim checks the family reading on a trial Call
  (elim_trial), then the Unit or Sum reading. If both fail, the family
  failure stays. New: examples/elim.lang and the elim-* rows.
- Part 2 (sessions 6 and 7): check.c has the SIGNATURE rows outcomeSet
  and menu (SIG_SLOT) and OP_SLOTS (actChoice {G, Choice}, outcomeSet
  {D}, menu {D} with kernel Constitution). front.c `law2_source`
  (orbitEq, orbitSet, descentPoint, descentUnique) goes after the last
  of LAW2_AFTER (IsSelfConstituting, outcomeSet), after the ledger
  splice. descentUnique is an Orbit elim into `Path D (L z) (K z)`: the
  quotEq case is a PathP by outcomeSet, and the quotSquash case is a
  square by isSet D. examples/dao.lang has outcomeSet, L1 and law2.
- Part 3 (sessions 8 to 11): amendPath is SIG_DEFINED, thus the guest
  term is amendExit (USER ruling, session 8). With the fast path, the
  M face with the ledger face took more than 20000000 eval steps
  (EVAL_FUEL). Root cause (record S8-s9-diag.md): the loop step of
  conv_loop compares the last argument of an op without the two fast
  tests of conv_values, thus the Prod type of the last Config field
  compares the unfolded Glue types of LendingLedger. Fix
  (S8-s10-fix.patch, +7 lines): after the switch, if a == b or
  conv_head, conv_loop returns 1. The M face with the ledger face then
  takes 98155 steps. The port: S8-o27b.patch (eval.c, core.h),
  S8-s10-fix.patch and amendExit (S8-s11-resolve.py).
- Session 12: the kill rows for m2 and m3 (USER rulings, session 11).
  examples/dao.lang has `def menu : D -> Constitution := fun (d : D) =>
  c0` after c0. test/check/menu-early.lang defines menu before the DAO
  prelude (REFUSE_SIG). The Part 1 mutants p1 to p4 run.

| Row | Code | PLAN gate row |
|---|---|---|
| elim-family-count | TYPE_MISMATCH | B1 |
| elim-path-case | PATH_END | B1 |
| elim-motive-index | TYPE_MISMATCH | B1 |
| elim-block | TYPE_MISMATCH | B1 |
| elim-family-prop | NAME_UNKNOWN | B1 |
| outcome-set-type | REFUSE_SIG | a slot at a wrong type |
| menu-type | REFUSE_SIG | a slot at a wrong type |
| menu-early | REFUSE_SIG | menu before the DAO prelude (the m3 kill row) |
| dao-no-set | NAME_UNKNOWN | a law 2 use without outcomeSet |
| dao-redefine | REFUSE_SIG | a redefined lend SIG_DEFINED name |
| amend-end | PATH_END | a wrong amendExit end |

Eval rows: elim bagTwoCountE, replicateTwo, swapAt0, swapAt1, bagSwapE,
quotThree, quotEqAt and hcompE (Part 1); dao law2At0 and law2At1, both
`inl unit` (Part 2); dao amendHold0 `inl (pair unit unit)`, amendHold1
`inr (inl (pair unit unit))` and amendOther1 `inr (inr unit)` (Part 3).

### Mutants

~/Documents/lend-lang-work/S8-s11-mutants.py (m1 to m7, session 11) and
S8-s12-mutants.py (m1 to m7 again and p1 to p4, session 12), in the copy
S8-s11-tree. Each mutant is an exact replace (anchor count 1), then
`make -B check`, then a restore. Logs: S8-s11-mutants.out and
S8-s12-p1-mutants.out.

| Id | Target | Change | Result | Killer |
|---|---|---|---|---|
| m1 | check.c OP_SLOTS outcomeSet | the row name is outcomeSetX | KILLED | dao.lang REFUSE_SIG; dao law2At0, law2At1 |
| m2 | check.c OP_SLOTS menu | the row name is menuX | KILLED (session 12) | dao.lang REFUSE_SIG at menu ("an op slot type"); dao law2At0. Session 11: SURVIVED |
| m3 | check.c slot_ok kernel test | `0 &&` before find_def Constitution | KILLED (session 12) | menu-early: REFUSE_REC in place of REFUSE_SIG. Session 11: SURVIVED |
| m4 | front.c LAW2_AFTER | no "outcomeSet" | KILLED | dao.lang REFUSE_REC descentUnique; ledger, orbit NAME_UNKNOWN |
| m5 | front.c law2 splice | `(1 \|\| splice_after_all(...))` | KILLED | dao.lang NAME_UNKNOWN descentUnique; dao law2At0, law2At1 |
| m6 | eval.c conv_loop fast test | the 2 lines deleted | KILLED | dao.lang, amend-end, dao law2At0: EVAL_FUEL amendExit |
| m7 | eval.c conv_head argument test | `0 &&` before conv_values | KILLED | ua-not-refl, ua-two-proofs admitted (PATH_UIP) |
| p1 | check.c elim_on_family motive arity | `0 &&` before the index_count test | KILLED | elim-motive-index: INTERNAL (an evaluator error) in place of TYPE_MISMATCH |
| p2 | check.c rule_elim candidate fallback | the Unit or Sum trial never runs | KILLED | fronttool domain (Switch): TYPE_MISMATCH in place of ok |
| p3 | check.c fold_image path-case IH end | the end is v, not the IH variable | KILLED | dao.lang PATH_UIP descentUnique; elim.lang PATH_END toTruncE; elim evals |
| p4 | check.c fold_image square-face IH path | the face is v, not the IH path | KILLED | dao.lang PATH_UIP descentUnique; elim.lang PATH_UIP toTruncE; elim evals |

The loop kills 11 of 11 mutants with 1 admission mutant (m7). In
session 11, try 1 of m1 and m2 (the row type swapped to the other SIG_
term) and of m5 (the line deleted) left a static function unused. The
clang check refused it, thus these tries are INVALID, and the table has
try 2. A site study (read only, session 12) found no single edit in the
S8-only elim code that admits a program the pristine checker refuses:
p2 to p4 refuse admitted programs, and p1 changes the refusal class. No
log has a compile error. m0-restored is GREEN, and the copy src equals
the repo src after each loop.

### Builders

| Stage | Tier | Result |
|---|---|---|
| Parts 0 to 3 | wf-builder Fable xhigh | died in session 1, 429, before a file change |
| survey | wf-mechanical | session 1; a hook blocked its write, the main loop wrote S8-survey.md |
| Part 0 probe | wf-builder opus xhigh | session 2: stopped at the agent call budget before the probe; the main loop ran its probe script |
| Part 1 probe | wf-builder Fable xhigh, then opus xhigh | died in session 3 (429; `reasoning_extraction`), before a file change |
| Part 1 site study | wf-mechanical | session 12, read only |
| session 12 port, repo gate | wf-closer | session 12 |
| records | wf-builder Fable xhigh | died in session 12, `reasoning_extraction`, before a file change |
| sessions 1 to 13 | main loop | by hand |

## S9

S9 builds the ZK ledger on the S8 tree, in the part order of the USER
rulings (S9, 2026-10-19): the port of orbit-lang c3ab788, the host
Pedersen commitment on BN254 G1 and the Sigma prover with a known-answer
fixture (test/zktool.c), the zk prelude text and the guest terms
(examples/zk.lang), the lowering and the chain rows (test/zk.sh), and
zkEquiv (O14) in its own part. PLAN section 8, "Builder call (S9,
2026-10-19)", has the rulings, the defaults and the result paragraphs.

### Counts

| Gate line | lend-lang S8 | lend-lang S9 |
|---|---|---|
| examples | 35 | 37 |
| parse refusals | 19 | 19 |
| check refusals | 181 | 186 |
| kit-only refusals | 22 | 24 |
| evals | 101 | 139 |
| runs, run refusals | 5, 10 | 5, 10 |
| asmtool checks | 8 | 8 |
| builds | 3 | 4 |
| lower refusals | 16 | 16 |
| build checks | 33 | 34 |
| dispatch | 20 | 20 |
| evm steps | 43 | 43 |
| precompile | not in the S8 table | 24 |
| zk chain steps | not in the S8 table | 56 |

The S9 column is ~/Documents/lend-lang-work/S9-p5-gate.log (exit 0).
The part logs are S9-p1-gate.log to S9-p5-gate.log in the same
directory.

### Parts

- Part 1 (session 2): the port of orbit-lang c3ab788, by hand. Record:
  ~/Documents/lend-lang-work/S9-p1-record.md.
- Part 2 (session 3): the host Pedersen and Sigma prover, with 23 checks
  in test/zktool.c. Record: S9-p2-record.md.
- Part 3 (sessions 6 to 8, options (a), (b) and (c)): the zk prelude
  text and the guest terms, with the new internal op u256AddMod (the
  split `e0 + e1 = e (mod r)` of the range proof, lowered to ADDMOD).
  Record: S9-p3-record.md.
- Part 4 (sessions 10 and 11): the Part 4 entries of examples/zk.lang
  and the chain gate test/zk.sh (56 steps: the 32-bit range chain with
  zkRangeBit and zkRangeDone, then zkRepay and zkLiquidate). Record:
  S9-p4-record-part.md.
- Part 5 (sessions 13 to 15): the sgPathP fix, the probe and the OOM,
  then the kernel edit in src/front/eval.c def_value (the with_head form
  for every origin, not only ORIGIN_DOMAIN), the block appended to
  examples/zk.lang (lines 187 to 323), 4 eval rows (136 to 139) and
  test/check/zk-forward-witness.lang (PATH_END). Record:
  S9-p5-record.md.

### Mutants

| Part | Result | Mutants |
|---|---|---|
| 4 | 6 of 6 KILLED (S9-p4-mutants.log) | wrong opening (zkDlEq without the e.C term), range overflow in the prelude (zkRangeStep without bitVerify), range overflow on the chain (zkRangeBit without bitVerify), non-member (zkBorrow without zkPosActive), Forfeited (zkPosActive without the status = 1 test), a point not on the curve (g1_from_words without g1_on_curve) |
| 5 | 3 of 3 KILLED (S9-p5-mutants.log) | (1) the admission mutant: the ORIGIN_DOMAIN head restored, so `--kit check examples/zk.lang` reports OOM; (2) and (3) in the record |

The records of Parts 1 to 3 have no mutant run. The Part 4 list is the
mutant list of the S9 row of PLAN section 8.

### Builders

Both builder tiers died on the classifier in sessions 1 and 2, before
the Part 1 and Part 2 builds. The main loop built Parts 1 to 3 by hand
(the record heads) and made the Part 5 kernel edit and build in
sessions 14 and 15. S9-p4-record-part.md has the Part 4 route.

## S10

S10 runs the contract end to end on the S9 tree: Loan and zkPrivateLoan,
the pure ledger borrow, repay and liquidate, the host and EVM
differential with the gas table and the EIP size rows, the ledger and
contract integration, and the governed admit (propose, vote, amend, the
self-exit of the caller). The S10 note of PLAN section 7 gives the S10
form of vote and amend. The close part (sessions 11 and 12, 2026-10-20)
is docs only: the size rows and the measured deploy gas after Part 5,
the S10 notes of O12, O17, O19 and O22, the S10 close paragraph of
docs/VALIDATION.md, and the new rows S11 (the governed exit, then the
terms menu) and S12 (O12 (a)). The old S11 is S13. The chain probe is a
USER step (O22).

### Counts

| Gate line | lend-lang S9 | Part 1 | Part 2 | Part 3 | Part 4 staged | Part 4 commit e988ab5 | Part 5 = 48a6b86 |
|---|---|---|---|---|---|---|---|
| check refusals | 186 | 189 | 193 | 193 | 193 | 193 (h) | 193 |
| evals | 139 | 142 | 150 | 150 | 156 | 166 | 183 |
| runs | 5 | 5 | 5 | 6 | 6 | 6 | 6 |
| build checks | 34 | 34 | 34 | 36 | 36 | 36 | 36 |
| zk chain steps | 56 | 56 | 56 | 61 | 65 | 78 | 114 |
| diff rows | 15 | 15 | 15 | 16 | 16 | 16 | 16 |

The other lines are equal in all columns: examples 37, parse refusals
19, kit-only refusals 24, run refusals 10, builds 4, lower refusals 16,
asmtool 8, dispatch 20, evm steps 43, precompile 24, mock 13, emit 27,
zk 33. The Part 1 to Part 4 columns are the gate logs of the staged
trees. The e988ab5 column is the session 9 baseline
(~/Documents/lend-lang-work/S10-p5-brief.md section 1). (h) is a
hypothesis: Part 5 adds no check row, so e988ab5 has 193. No gate log
of bf2c697 or 04174b8 exists. The close tree changes docs only; its gate
log is ~/Documents/lend-lang-work/S10-close-gate.log.

### Parts

| Part | Scope | Session, record | Staged tree | USER commit (tree) |
|---|---|---|---|---|
| 1 | Loan, zkPrivateLoan | 2, S10-p1-record.md | 1ea99a67 | 7c18f30 (1ea99a67, equal) |
| 2 | ledger borrow, repay, liquidate | 4, S10-p2-record.md | f0748ade | bf2c697 (22ba30cc, differs: +184/-1, staged +167/-1) |
| 3 | differential, gas table, EIP rows | 6, S10-p3-record.md | 261fc937 | 04174b8 (e882801c, differs) |
| 4 | ledger/contract integration | 8, S10-p4-record.md | 637c0a20 | e988ab5 (2da59d26, differs: +zkNegScalar, +zkRepayDeadline) |
| 5 | governed admit | 10, S10-p5-record.md | d431c76d | 48a6b86 (d431c76d, equal) |

The records are in ~/Documents/lend-lang-work. The USER commits of
Parts 2, 3 and 4 differ from the staged trees; the records give the
staged numbers.

### Mutants

50 of 50 KILLED, on the staged trees: Part 1 3 of 3, Part 2 9 of 9,
Part 3 11 of 11, Part 4 11 of 11, Part 5 16 of 16. The USER deltas of
Parts 2 to 4 have no mutant record. The close part runs no mutant (docs
only).

### Builders

| Stage | Tier | Result |
|---|---|---|
| Parts 1 to 5 | see the part records | S10-p1-record.md to S10-p5-record.md |
| close brief and close (sessions 11 and 12) | main loop | by hand, no builder call |

## S11

S11 adds the governance that the S10 close moved out of S10 (USER
ruling 2026-10-20). Part 1 adds the governed exit: a poll kind (admit
or exit) at slot 15, zkProposeExit and the zkAmend exit branch. An exit
withdraws the vote of the open poll. Part 2 adds the terms menu (form
B): terms and chg at slots 16 and 17, zkProposeTerms and zkTerms, the
zkAmend terms branch, the menu deadline in zkBorrow and the O17 window
in zkLiquidate. S11 does not add voter anonymity (O12 (a) is S12). The
S11 paragraphs of docs/VALIDATION.md give the rows and the gas table.

### Counts

| Gate line | S10 close c8d8afd | Part 1 = 0c64d36 | Part 2 = f955b71 |
|---|---|---|---|
| evals | 183 | 183 | 183 |
| zk chain steps | 114 | 192 | 249 |

The other lines are equal in all columns: examples 37, parse refusals
19, check refusals 193, kit-only refusals 24, runs 6, run refusals 10,
asmtool 8, builds 4, lower refusals 16, build checks 36, dispatch 20,
evm steps 43, precompile 24, mock 13, emit 27, zk 33, diff rows 16. The
c8d8afd column is the S10 Part 5 column (the close is docs only). The
Part 1 column is ~/Documents/lend-lang-work/S11-p1-gate.log. No gate
log of the Part 2 tree is in lend-lang-work: the Part 2 zk chain count
is from docs/VALIDATION.md (192 to 249 steps), and the other lines are
from S12-s2-gate1.log, the first S12 gate, which has no S12 row yet
(h).

### Parts

| Part | Scope | Session, record | Staged tree | USER commit (tree) |
|---|---|---|---|---|
| 1 | governed exit | 1 and 2, S11-p1-brief.md, S11-p1-gate.log, S11-p1-mutants.log | 7963e1fa | 0c64d36 (7963e1fa, equal; the subject is reworded) |
| 2 | terms menu, O17 window | 3 and 4, S11-p2-brief.md, S11-p2-probe-s3, S11-p2-mut-s4 | 48d72d34 | f955b71 (a15992ad, differs: docs/PLAN.md, docs/VALIDATION.md, examples/zk.lang, +9/-5) |

The records are in ~/Documents/lend-lang-work. S11 has no record file:
the briefs, the gate log and the mutant logs are the record.

### Mutants

15 of 15 KILLED, on the staged trees: Part 1 7 of 7
(S11-p1-mutants.log; a REFUSE_REC kills M6, and 12 zk chain failures
kill its variant M6b), Part 2 8 of 8 (S11-p2-mut-s4). The USER delta of
Part 2 has no mutant record.

### Builders

| Stage | Tier | Result |
|---|---|---|
| Parts 1 and 2 | no builder record | S11-p1-brief.md, S11-p2-brief.md |

## S12

S12 adds the ring vote of O12 (a) in Form A (rulings Q-S12-1 to
Q-S12-7 (a), D1 to D3 (a), K = 4): zkKey stores the key of a member,
zkVote checks a bLSAG ring signature over the stored keys, zkDepart
removes the key of the sender, and the zkAmend bar uses nmem0 (Q-S12-3
(a)). The USER commit 59bb780 adds two items to the staged tree: the
explicit deployment domain (zkDomain, set once by the founder, with its
setup guards and replay rows) and the shared conditional words. The S12
paragraph of docs/VALIDATION.md gives the rows, the gas table and the
sizes.

### Counts

| Gate line | S11 = f955b71 | staged 7e86f50f | USER commit 59bb780 |
|---|---|---|---|
| evals | 183 | 184 | 190 |
| zk chain steps | 249 | 278 | 294 |

The other lines are equal in all columns: examples 37, parse refusals
19, check refusals 193, kit-only refusals 24, runs 6, run refusals 10,
asmtool 8, builds 4, lower refusals 16, build checks 36, dispatch 20,
evm steps 43, precompile 24, mock 13, emit 27, zk 33, diff rows 16. The
f955b71 column is S12-s2-gate1.log. The 7e86f50f column is
S12-s8-gate.log. The 59bb780 column uses the counts of the commit
message (190 evals, 294 chain steps), not the staged counts. No gate
log of 59bb780 is in lend-lang-work, thus its other lines are (h). The
S13 gate 1 agrees: it has the 59bb780 counts plus the S13 rows.

### Parts

| Part | Scope | Session, record | Staged tree | USER commit (tree) |
|---|---|---|---|---|
| 1 | ring vote: zkKey, zkVote Form A, the key removal, the nmem0 bar | 1 to 8, S12-brief.md, S12-s2-HANDOFF.md to S12-s8-HANDOFF.md | 7e86f50f | 59bb780 (5cf02c7f, differs: +deployment domain, +shared conditional words; 8 files, +176/-83) |

### Mutants

8 of 8 KILLED, on the staged tree before the domain review fix: M1 to
M8 (S12-mut-s3; the S12 paragraph of docs/VALIDATION.md gives the
killer of each). M7 (ix != 0) is equivalent on chain states; the
white-box eval row forgeVoteO and the 4 zk.sh zkVote gas rows kill it.
The USER delta has no mutant record. Six host eval regressions and the
setup and replay rows of the zk chain check it.

### Builders

| Stage | Tier | Result |
|---|---|---|
| sessions 1 to 8 | no builder record | runners in S12-s3-runners to S12-s8-runners |

## S13

S13 closes the plan: the regimes of O23 (ruling (a), USER 2026-10-21),
the docs and the final gate. The regimes are the port of orbit-lang S10
B1 with the lend names (O25: lend-lang does not change orbit-lang):
examples/regimes.lang (guest mode), the Empty family in
domain/domain.lang, 4 check rows and 6 eval rows. The Q-S13-1 (a)
probe passed: the guest `aggregate` by elim on Orbit checks, and the
laws paraLaw, forkLaw and noneLaw hold by refl. No fold form was
necessary. The docs part adds `## S11` to `## S13` here, the regimes in
`## Implemented` and `## Known limits`, the S13 row of PLAN section 8,
the O23 ruling and the S13 paragraph of docs/VALIDATION.md. S13 adds no
contract code. The chain probe stays a USER step (O22).

### Counts

| Gate line | S12 = 59bb780 | port, gate 1 | port and docs, gate 2 |
|---|---|---|---|
| examples | 37 | 38 | 38 |
| check refusals | 193 | 197 | 197 |
| evals | 190 | 196 | 196 |

The other lines are equal in all columns: parse refusals 19, kit-only
refusals 24, runs 6, run refusals 10, asmtool 8, builds 4, lower
refusals 16, build checks 36, dispatch 20, evm steps 43, precompile 24,
zk chain steps 294, mock 13, emit 27, zk 33, diff rows 16. Gate 1 is
~/Documents/lend-lang-work/S13-gate1.log (753 s); gate 2 is
S13-gate2.log. The port changes no test file outside test/check and
test/eval, thus the other lines of the 59bb780 column are (h).

### Parts

| Part | Scope | Session, record | Staged tree | USER commit (tree) |
|---|---|---|---|---|
| port | Empty, examples/regimes.lang, 4 check rows, 6 eval rows | 1 (brief: S13-brief.md, S13-s1-HANDOFF.md) and 2 (probe, port, mutants, gate 1: S13-s2-HANDOFF.md) | bfb92e9b | with the docs |
| docs | STATUS S11 to S13, Implemented, Known limits, PLAN S13 row and O23, VALIDATION S13 | 3, S13-s3-HANDOFF.md | the final tree (S13-s3-HANDOFF.md) | not made yet |

### Mutants

6 of 6 KILLED, on the port tree (session 2). Each mutant also fails the
first 6 eval rows, because `langc eval` checks the whole file first.

| Mutant | Site | Change | First failed line (killer) |
|---|---|---|---|
| MR1 | `bump` | the first fixed point gives SI, not AD | examples gate, `paraLaw` PATH_END |
| MR2 | `betaGt` | the zero case gives inl unit | examples gate, `paraLaw` PATH_END |
| MR3 | `isFix` | compares to phaseZero, not o | examples gate, `paraLaw` PATH_END |
| MR4 | `nextPhase` | phaseMinus selects itself | examples gate, `noneLaw` PATH_END |
| MR5 | `Empty` (domain/domain.lang) | a base constructor `emptyBase` | examples gate, `exfalso` TYPE_MISMATCH |
| MR6 | `aggregate`, quot case | phaseZero, not `witness F x` | examples gate, `aggregate` TYPE_MISMATCH |

The S13 brief expected `forkLaw` PATH_END for MR6. The real killer is
the elim check of `aggregate`: the ends of the quotEq case are not
phaseZero. The docs part runs no mutant (docs only).

### Builders

| Stage | Tier | Result |
|---|---|---|
| sessions 1 and 2 (brief, probe, port, mutants, gate 1) | main loop | by hand, runners in S13-s1-runners and S13-s2-runners |
| STATUS S11 and S12 (session 3) | wf-builder at fable xhigh, then one opus xhigh re-run | both stopped (a rate limit, then a classifier stop); main loop by hand |
| the other docs (session 3) | main loop | by hand, no builder call |
