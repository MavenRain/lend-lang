#!/bin/sh
# The zk chain gate (slice S9, Part 4). Deploy examples/zk.lang, then do one
# `evm run --prestate` step for each row: the known-answer views (the commitment
# words, the Schnorr and the bit verifiers with the proof words as parameters,
# so the curve ops stay open and lower to the 0x06/0x07 precompiles), the five
# reverts of PLAN section 8, the range chain (32 zkRangeBit steps, rb31 first,
# then zkRangeDone) and the flow rows (zkRepay, zkLiquidate at timestamps 8 and
# 9, then a Forfeited zkBorrow). The words are the words of the zk.lang
# literals, read by `langc --kit eval` (the same words as the eval rows): this
# file carries no literal. It needs evm (go-ethereum), bc, od, build/slottool
# and build/langc. Run it from the kit root.
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
  # The reverts of PLAN section 8. The caller aa enrolls (ungoverned in S9).
  call ok zkEnroll "$caller"
  # 1. wrong opening: schnorrKABad has s1 + 1.
  call revert zkBorrow $(words cKA) $(words schnorrKABad)
  # 3. non-member: bb never enrolled.
  callAt 1 "$other" revert zkBorrow $(words cKA) $(words schnorrKA)
  # 4. Forfeited: depart, then borrow reverts; enroll again to continue.
  call ok zkDepart "$caller"
  call revert zkBorrow $(words cKA) $(words schnorrKA)
  call ok zkEnroll "$caller"
  # 5. a point not on the curve: (1, 1), the 0x07 STATICCALL fails (R18).
  call revert zkBorrow 1u 1u $(words schnorrKA)
  # The opening: zkBorrow with cKA and the Schnorr proof, the deadline 1 + 7 = 8.
  call ok zkBorrow $(words cKA) $(words schnorrKA)
  # 2. range overflow: rb00Bad has e0 + 1, and the end before any bit.
  call revert zkRangeBit $(words rb00Bad)
  call revert zkRangeDone
  # The range chain: the 32 bit rows, rb31 (the top bit) first, then the end.
  for n in $(seq 31 -1 0); do
    call ok zkRangeBit $(words "$(printf 'rb%02d' "$n")")
  done
  call ok zkRangeDone
  # The flow rows: repay clears the deadline, so no liquidation; borrow again
  # (the deadline 8), liquidate at 8 reverts and at 9 forfeits; then borrow
  # reverts (Forfeited).
  call ok zkRepay $(words cKA) $(words schnorrKA)
  callAt 9 "$caller" revert zkLiquidate "$caller"
  call ok zkBorrow $(words cKA) $(words schnorrKA)
  callAt 8 "$caller" revert zkLiquidate "$caller"
  callAt 9 "$caller" ok zkLiquidate "$caller"
  call revert zkBorrow $(words cKA) $(words schnorrKA)
fi

rm -rf "$tmp"
echo "zk chain: $steps steps checked"
echo "zk chain gate: $fail failures"
[ "$fail" -eq 0 ]
