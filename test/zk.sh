#!/bin/sh
# The zk chain gate (slice S9, Part 4). Deploy examples/zk.lang, then do one
# `evm run --prestate` step for each row: the known-answer views (the commitment
# words, the Schnorr and the bit verifiers with the proof words as parameters,
# so the curve ops stay open and lower to the 0x06/0x07 precompiles), the
# governed admit of S10 Part 5 (zkPropose, zkVote, zkAmend, the self-exit
# zkDepart and the view zkIsMember; the founder aa is the only member at
# deploy), the reverts of PLAN section 8, the range chain (32 zkRangeBit
# steps, rb31 first, then zkRangeDone) and the flow rows (zkRepay,
# zkLiquidate at timestamps 8 and 9, then a Forfeited zkBorrow). The words
# are the words of the zk.lang
# literals, read by `langc --kit eval` (the same words as the eval rows): this
# file carries no literal. It needs evm (go-ethereum), bc, od, build/slottool
# and build/langc. Run it from the kit root. Twenty-one gas rows (S10 Parts 3
# and 5, S11 Parts 1 and 2, gasRow) check the execution gas of the first zkPropose, the
# first yes zkVote, the admit zkAmend, the first zkBorrow, the last
# zkRangeBit, zkRangeDone, zkRepay, the ok zkLiquidate, the first
# zkProposeExit and the three exit zkAmend rows (expel, reject, and the close
# after the candidate exits) and the 9 terms rows of S11 Part 2 against the gas
# table of docs/PLAN.md section 6 and docs/VALIDATION.md,
# exact: an emitter change moves them on purpose.
# S10 Part 4: zkBorrow and zkRepay take the public delta d and the blinding r
# (two words: no proof and no point), so the wrong-opening and off-curve rows
# leave. The delta rows of bb (two borrows, a partial repay and an excess
# repay) and the third-party enroll of the Forfeited aa (a revert) close the
# block.
# S11 Part 1, the governed exit, runs on a second deploy: zkProposeExit, the
# exit zkAmend (expel, reject, the close after the candidate exits) and the F7
# rows (an exit withdraws the caller's vote of the open poll).
# S11 Part 2, the terms menu, runs on a third deploy: zkProposeTerms, the terms
# zkAmend (apply, reject, and the F7 withdraw), the view zkTerms, the zkBorrow
# deadline of the menu entry and the O17 window in zkLiquidate.
set -u
fail=0
steps=0
tmp=${TMPDIR:-/tmp}/langc-zk.$$
mkdir -p "$tmp"
. test/evmchain.sh

prog=examples/zk.lang
caller=0x00000000000000000000000000000000000000aa
other=0x00000000000000000000000000000000000000bb

# words NAME: the words of the zk.lang literal NAME, one per line, in pair
# order: a commitment is x y, a Schnorr proof is ax ay s1 s2 and a range row is
# cx cy a0x a0y a1x a1y e0 e1 s0 s1 (the test/build.sh idiom).
words() {
  build/langc --kit eval "$prog" "$1" | tr -d '()' | awk '{ for (i = 1; i <= NF; i++) if ($i != "pair") print $i }'
}

# callAt NOW SENDER WANT NAME ARGS...: one step of the view or entry NAME with
# ARGS from SENDER at TIMESTAMP NOW. The result must equal WANT: `out HEX` (HEX
# as hexnorm), `ok` or `revert`. An entry changes the storage of the next step.
callAt() {
  _now=$1 _sender=$2 _want=$3 _name=$4
  shift 4
  steps=$((steps + 1))
  _input=$(input "$prog" "$_name" "$@")
  if ! step "$_now" "$_sender" "$_input"; then
    echo "FAIL zk $_name: evm: $(head -n 1 "$tmp/dump")"
    fail=$((fail + 1))
    return
  fi
  case $result in
    "out "*) _hex=${result#out }
      [ ${#_hex} -eq 64 ] && result="out $(hexnorm "$_hex")" ;;
  esac
  if [ "$result" != "$_want" ]; then
    echo "FAIL zk $_name: want $_want, got $result"
    fail=$((fail + 1))
  fi
}

# call WANT NAME ARGS...: callAt from the caller aa at TIMESTAMP 1.
call() {
  callAt 1 "$caller" "$@"
}

# gasRow WANT: the execution gas of the last step (gas of step, evm
# --statdump) must be WANT. A gas row counts as a step.
gasRow() {
  steps=$((steps + 1))
  [ "${gas:-}" = "$1" ] || { echo "FAIL zk gas $_name: want $1, got ${gas:-none}"; fail=$((fail + 1)); }
}

# admit PROPOSER CAND VOTER...: the governed admit of S10 Part 5. PROPOSER
# opens the poll on CAND, each VOTER votes yes, then PROPOSER amends (the
# admit needs 2 ayes > nmem). Each call is a step.
admit() {
  adm_p=$1
  adm_c=$2
  shift 2
  callAt 1 "$adm_p" ok zkPropose "$adm_c"
  for adm_v in "$@"; do
    callAt 1 "$adm_v" ok zkVote 1
  done
  callAt 1 "$adm_p" ok zkAmend
}

if ! deploy "$prog"; then
  echo "FAIL zk deploy: $(head -n 1 "$tmp/dump")"
  fail=$((fail + 1))
else
  # The known-answer views. cKA = commitWords (7, 11): the x word and the y word.
  cx=$(words cKA | head -n 1)
  cy=$(words cKA | tail -n 1)
  call "out $(word "$cx")" zkCommitX 7u 11u
  call "out $(word "$cy")" zkCommitY 7u 11u
  call "out 1" zkSchnorrView $(words cKA) $(words schnorrKA)
  call "out 0" zkSchnorrView $(words cKA) $(words schnorrKABad)
  call "out 1" zkBitView $(words c0) $(words bitProof0)
  call "out 1" zkBitView $(words c1) $(words bitProof1)
  # bitBad: the bit 1 proof against the bit 0 commitment.
  call "out 0" zkBitView $(words c0) $(words bitProof1)
  # S10 Part 5, the governed admit. aa is the founder (init): the only member
  # at deploy, nmem 1. ee and ff are the candidates. A non-member cannot
  # propose. zkIsMember is the view.
  ee=0x00000000000000000000000000000000000000ee
  ff=0x00000000000000000000000000000000000000ff
  callAt 1 "$other" revert zkPropose "$ee"
  call "out 1" zkIsMember "$caller"
  call "out 0" zkIsMember "$ee"
  call ok zkPropose "$ee"
  gasRow 28948
  # One poll at a time, no amend before a decision, and a vote is one member,
  # 0 or 1, once in each poll.
  call revert zkPropose "$ff"
  call revert zkAmend
  callAt 1 "$other" revert zkVote 1
  call revert zkVote 2
  call ok zkVote 1
  gasRow 54023
  call revert zkVote 1
  # 2 ayes > nmem (2 x 1 > 1): the amend admits ee, nmem 2.
  call ok zkAmend
  gasRow 86983
  call "out 1" zkIsMember "$ee"
  # ee proposes ff. One aye of two members (2 x 1 = nmem) does not admit;
  # then ee votes no and 2 nays >= nmem rejects ff. No poll is open after it.
  callAt 1 "$ee" ok zkPropose "$ff"
  call ok zkVote 1
  call revert zkAmend
  callAt 1 "$ee" ok zkVote 0
  call ok zkAmend
  call "out 0" zkIsMember "$ff"
  call revert zkAmend
  # The self-exit: ee leaves (Forfeited, nmem 1) and cannot propose again.
  callAt 1 "$ee" ok zkDepart
  call "out 0" zkIsMember "$ee"
  callAt 1 "$ee" revert zkPropose "$ff"
  # The reverts of PLAN section 8. The founder aa is a member from deploy.
  # 1. (a wrong opening) and 5. (a point off the curve) leave in S10 Part 4:
  # zkBorrow takes the delta words only.
  # 3. non-member: bb is not a member yet.
  callAt 1 "$other" revert zkBorrow 7u 11u
  # 4. Forfeited: aa admits bb, then aa exits (zkDepart) and borrow reverts;
  # bb admits aa again to continue.
  admit "$caller" "$other" "$caller"
  call ok zkDepart
  call revert zkBorrow 7u 11u
  admit "$other" "$caller" "$other"
  # The first borrow: zkBorrow 7u 11u from the identity stores cKA, the deadline
  # 1 + 7 = 8.
  call ok zkBorrow 7u 11u
  gasRow 109504
  # 2. range overflow: rb00Bad has e0 + 1, and the end before any bit.
  call revert zkRangeBit $(words rb00Bad)
  call revert zkRangeDone
  # The range chain: the 32 bit rows, rb31 (the top bit) first, then the end.
  for n in $(seq 31 -1 0); do
    call ok zkRangeBit $(words "$(printf 'rb%02d' "$n")")
  done
  gasRow 60942
  call ok zkRangeDone
  gasRow 46800
  # The flow rows: the full repay (7u 11u, back to the identity) clears the
  # deadline, so no liquidation; borrow again
  # (the deadline 8), liquidate at 8 reverts and at 9 forfeits; then borrow
  # reverts (Forfeited).
  call ok zkRepay 7u 11u
  gasRow 45467
  callAt 9 "$caller" revert zkLiquidate "$caller"
  call ok zkBorrow 7u 11u
  callAt 8 "$caller" revert zkLiquidate "$caller"
  callAt 9 "$caller" ok zkLiquidate "$caller"
  gasRow 12323
  call revert zkBorrow 7u 11u
  # S10 Part 4, the delta rows: bb (a member since row 4.) borrows 5u 13u from
  # the identity and 7u 13u on top, repays 5u 13u, then repays 20u 13u, more
  # than the 7u it owes: accepted, the position wraps mod q (docs/PLAN.md, S10
  # Part 4). Last, bb cannot propose aa: zkLiquidate keeps the member flag.
  callAt 1 "$other" ok zkBorrow 5u 13u
  callAt 1 "$other" ok zkBorrow 7u 13u
  callAt 1 "$other" ok zkRepay 5u 13u
  callAt 1 "$other" ok zkRepay 20u 13u
  callAt 1 "$other" revert zkPropose "$caller"
  # Zero, partial and excess repayments keep the original deadline. A
  # liquidated holder stays a member: no member can propose it.
  callAt 9 "$other" ok zkLiquidate "$other"
  call revert zkPropose "$other"
  third=0x00000000000000000000000000000000000000cc
  # aa and bb admit cc (2 x 2 > nmem 2).
  admit "$caller" "$third" "$caller" "$other"
  callAt 1 "$third" ok zkBorrow 7u 11u
  callAt 1 "$third" ok zkRepay 0u 0u
  callAt 1 "$third" ok zkRepay 5u 5u
  callAt 8 "$third" revert zkLiquidate "$third"
  callAt 9 "$third" ok zkLiquidate "$third"
  callAt 1 "$third" revert zkPropose "$third"
  # ecMul reduces U256 scalars modulo q. Repayment must do the same before
  # negation, including both public arguments above q.
  fourth=0x00000000000000000000000000000000000000dd
  aboveq=21888242871839275222246405745257275088548364400416034343698204186575808495618u
  # aa and bb admit dd (2 of 3).
  admit "$caller" "$fourth" "$caller" "$other"
  callAt 1 "$fourth" ok zkBorrow "$aboveq" "$aboveq"
  callAt 1 "$fourth" ok zkRepay "$aboveq" "$aboveq"
  callAt 9 "$fourth" revert zkLiquidate "$fourth"
  # The Part 4 guard in zkPropose: cc (liquidated, the deadline 8 open)
  # exits, and no member can propose cc again.
  callAt 1 "$third" ok zkDepart
  call revert zkPropose "$third"
  # S11 Part 1, the governed exit (kind 1), on a second deploy: aa is again
  # the only member. f1, f2 and f3 are more candidates.
  if ! deploy "$prog"; then
    echo "FAIL zk deploy (S11 Part 1): $(head -n 1 "$tmp/dump")"
    fail=$((fail + 1))
  else
    f1=0x00000000000000000000000000000000000000f1
    f2=0x00000000000000000000000000000000000000f2
    f3=0x00000000000000000000000000000000000000f3
    admit "$caller" "$other" "$caller"
    admit "$caller" "$third" "$caller" "$other"
    admit "$caller" "$fourth" "$caller" "$other"
    call "out 1" zkIsMember "$fourth"
    # zkProposeExit reverts for a caller that is not a member and for a target
    # that is not a member (the zero address included).
    call revert zkProposeExit "$ee"
    callAt 1 "$ee" revert zkProposeExit "$fourth"
    call revert zkProposeExit 0x0000000000000000000000000000000000000000
    call ok zkProposeExit "$fourth"
    gasRow 49123
    # One poll at a time, for the two kinds.
    call revert zkPropose "$ee"
    call revert zkProposeExit "$third"
    # 2 ayes of 4 members (2 x 2 = nmem) do not expel; 3 ayes expel dd: not a
    # member, Forfeited, nmem 3.
    call ok zkVote 1
    callAt 1 "$other" ok zkVote 1
    call revert zkAmend
    callAt 1 "$third" ok zkVote 1
    call ok zkAmend
    gasRow 38909
    call "out 0" zkIsMember "$fourth"
    callAt 1 "$fourth" revert zkPropose "$ee"
    callAt 1 "$fourth" revert zkDepart
    # 2 nays of 3 members (2 x 2 >= 3) reject the exit of cc: cc stays.
    call ok zkProposeExit "$third"
    callAt 1 "$other" ok zkVote 0
    callAt 1 "$third" ok zkVote 0
    call ok zkAmend
    gasRow 25510
    call "out 1" zkIsMember "$third"
    # After an exit poll, an admit poll admits again (kind 0). Then ee exits
    # (zkDepart) during its own exit poll: the amend only closes the poll, and
    # nmem drops one time (4 to 3), not two times.
    admit "$caller" "$ee" "$caller" "$other"
    call ok zkProposeExit "$ee"
    call ok zkVote 1
    callAt 1 "$other" ok zkVote 1
    callAt 1 "$third" ok zkVote 1
    callAt 1 "$ee" ok zkDepart
    call ok zkAmend
    gasRow 27579
    # nmem is 3: one nay (2 x 1 < 3) does not reject f1, thus the amend
    # reverts (with nmem 2 it rejects). Then 2 ayes (2 x 2 > 3) admit f1.
    call ok zkPropose "$f1"
    call ok zkVote 0
    call revert zkAmend
    callAt 1 "$other" ok zkVote 1
    callAt 1 "$third" ok zkVote 1
    call ok zkAmend
    # F7, an admit poll: nmem 4 (aa bb cc f1). aa and bb vote yes on f2, then
    # the two exit. Each exit withdraws its vote: ayes 0, nmem 2, thus the
    # amend reverts. cc and f1 vote no and reject f2, then admit f2.
    call ok zkPropose "$f2"
    call ok zkVote 1
    callAt 1 "$other" ok zkVote 1
    call ok zkDepart
    callAt 1 "$other" ok zkDepart
    callAt 1 "$third" revert zkAmend
    callAt 1 "$third" ok zkVote 0
    callAt 1 "$f1" ok zkVote 0
    callAt 1 "$third" ok zkAmend
    admit "$third" "$f2" "$third" "$f1"
    # F7, an exit poll: nmem 4 (cc f1 f2 f3). cc and f1 vote yes on the exit
    # of f3, then the two exit: ayes 0, nmem 2, thus the amend reverts. f2
    # and f3 vote no and reject the exit: f3 stays.
    admit "$third" "$f3" "$third" "$f1"
    callAt 1 "$third" ok zkProposeExit "$f3"
    callAt 1 "$third" ok zkVote 1
    callAt 1 "$f1" ok zkVote 1
    callAt 1 "$third" ok zkDepart
    callAt 1 "$f1" ok zkDepart
    callAt 1 "$f2" revert zkAmend
    callAt 1 "$f2" ok zkVote 0
    callAt 1 "$f3" ok zkVote 0
    callAt 1 "$f2" ok zkAmend
    call "out 1" zkIsMember "$f3"
  fi
  # S11 Part 2, the terms menu, on a third deploy: aa, bb, cc and dd are the
  # members (nmem 4). The menu: entry 0 is zkGrace (7), entry 1 is 14, entry 2
  # is 3. The view zkTerms gives the menu index of the current terms.
  if ! deploy "$prog"; then
    echo "FAIL zk deploy (S11 Part 2): $(head -n 1 "$tmp/dump")"
    fail=$((fail + 1))
  else
    admit "$caller" "$other" "$caller"
    admit "$caller" "$third" "$caller" "$other"
    admit "$caller" "$fourth" "$caller" "$other"
    call "out 0" zkTerms
    # zkProposeTerms reverts for a caller that is not a member and for an
    # index that is not in the menu.
    callAt 1 "$ee" revert zkProposeTerms 1
    call revert zkProposeTerms 3
    # bb borrows at 1 under entry 0: the deadline is 8.
    callAt 1 "$other" ok zkBorrow 7u 11u
    call ok zkProposeTerms 1
    gasRow 47020
    # One poll at a time, for the three kinds.
    call revert zkPropose "$ee"
    call revert zkProposeExit "$third"
    call revert zkProposeTerms 2
    # 2 ayes of 4 members do not apply entry 1; 3 ayes apply it at 5 (chg 5).
    call ok zkVote 1
    gasRow 36923
    callAt 1 "$other" ok zkVote 1
    call revert zkAmend
    callAt 1 "$third" ok zkVote 1
    callAt 5 "$caller" ok zkAmend
    gasRow 69640
    call "out 1" zkTerms
    # The O17 window: no liquidation until now is past chg + zkGrace (12). The
    # deadline 8 of bb does not end the window.
    callAt 9 "$caller" revert zkLiquidate "$other"
    gasRow 9345
    callAt 12 "$caller" revert zkLiquidate "$other"
    callAt 13 "$caller" ok zkLiquidate "$other"
    gasRow 12323
    # dd borrows at 13 under entry 1: the deadline is 13 + 14 = 27.
    callAt 13 "$fourth" ok zkBorrow 7u 11u
    gasRow 109375
    callAt 27 "$caller" revert zkLiquidate "$fourth"
    callAt 28 "$caller" ok zkLiquidate "$fourth"
    # 2 nays of 4 members (2 x 2 >= 4) reject entry 2: the terms do not change.
    callAt 13 "$third" ok zkProposeTerms 2
    call ok zkVote 0
    callAt 1 "$other" ok zkVote 0
    call ok zkAmend
    gasRow 25548
    call "out 1" zkTerms
    # F7, a terms poll: aa and bb vote yes, then bb exits (ayes 1, nmem 3), thus
    # the amend reverts. cc votes yes, and the amend applies entry 2 at 20.
    call ok zkProposeTerms 2
    call ok zkVote 1
    callAt 1 "$other" ok zkVote 1
    callAt 1 "$other" ok zkDepart
    call revert zkAmend
    gasRow 11513
    callAt 1 "$third" ok zkVote 1
    callAt 20 "$caller" ok zkAmend
    gasRow 35440
    call "out 2" zkTerms
    # cc borrows at 20 under entry 2: the deadline is 23, but the window of the
    # change at 20 ends after 27.
    callAt 20 "$third" ok zkBorrow 7u 11u
    callAt 23 "$caller" revert zkLiquidate "$third"
    callAt 24 "$caller" revert zkLiquidate "$third"
    callAt 28 "$caller" ok zkLiquidate "$third"
  fi
fi

rm -rf "$tmp"
echo "zk chain: $steps steps checked"
echo "zk chain gate: $fail failures"
[ "$fail" -eq 0 ]
