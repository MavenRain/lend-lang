# lend-lang status

> Write here: one paragraph. Say which milestone the build implements, and
> which milestone it does not complete yet.

## Implemented

> Write here: one item for each feature that works. Name the former (F1 to
> F15), the core type or the core operation.

- <feature>.

## Remaining M0 work

> Write here: one numbered item for each missing feature. Say what each item
> needs first.

1. <work item>.

## Known limits

> Write here: one item for each limit of the current build, with its cause
> (a fuel budget, a host limit, a stack size).

- <limit>.

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
