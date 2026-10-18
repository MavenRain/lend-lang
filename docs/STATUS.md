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
