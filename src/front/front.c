#include <string.h>

#include "front/contract.h"
#include "front/front.h"

static int load_one(Arena *arena, const char *source, Origin origin, const char *text, size_t len, DeclList *out, Diag *diag) {
  TokenList toks;
  if (!lex_source(arena, source, text, len, &toks, diag)) return 0;
  return parse_source(arena, source, origin, &toks, out, diag);
}

/* Orbit as a kernel declaration (orbit-lang S4, PLAN section 8): the set
   quotient of the voter assignments Agent -> Choice by actChoice at each
   agent (lend-lang: the slot actChoice; act stays reserved). R is inline, so the kernel adds
   no other name. */
#define ORBIT_SOURCE_NAME "kernel/orbit.lang"
#define ORBIT_R "(fun (c : Agent -> Choice) (d : Agent -> Choice) => Sigma (g : G) (Path (Agent -> Choice) (fun (a : Agent) => actChoice g (d a)) c))"

static const char orbit_source[] =
  "def Orbit : Type 0 := Quot (Agent -> Choice) " ORBIT_R "\n"
  "def orbitProj : (Agent -> Choice) -> Orbit := fun (c : Agent -> Choice) => quot (Agent -> Choice) " ORBIT_R " c\n";

/* Membership as a kernel declaration (lend-lang S6, PLAN section 8): the
   member sets M : Agent -> 2 with 2 := Sum Unit Unit, 0 := inl unit and
   1 := inr unit. admit is a path from M to N when M a is 0, N a is 1 and N
   is M at each other agent (the premises P, Q and R), and exit is the same
   path with 0 and 1 swapped. There is no 2-cell (O6 (a)). The field names
   have the constructor name as prefix, so no field name is a global name. */
#define MEMBERSHIP_SOURCE_NAME "kernel/membership.lang"
#define MEMBERSHIP_STEP(name, from, to) \
  "  | " name " (" name "M : Agent -> Sum Unit Unit) (" name "N : Agent -> Sum Unit Unit) (" name "A : Agent)" \
  " (" name "P : Path (Sum Unit Unit) (" name "M " name "A) (" from " unit))" \
  " (" name "Q : Path (Sum Unit Unit) (" name "N " name "A) (" to " unit))" \
  " (" name "R : (" name "B : Agent) -> Sum (Path Agent " name "B " name "A) (Path (Sum Unit Unit) (" name "M " name "B) (" name "N " name "B)))" \
  " (i : I) [ ~i -> memberSet " name "M, i -> memberSet " name "N ]\n"

/* The point constructor is memberSet, not base: base is the S1 constructor
   (REFUSE_NAME, rule R4). */
static const char membership_source[] =
  "family Membership := memberSet (memberSetM : Agent -> Sum Unit Unit)\n"
  MEMBERSHIP_STEP("admit", "inl", "inr")
  MEMBERSHIP_STEP("exit", "inr", "inl");

/* The DAO prelude as kernel text (lend-lang S7, PLAN section 8, Part 1).
   Obj has no M index (USER ruling S7). IsSelfConstituting is
   IsFixedPointObj: the Sigma over L in Config is the witness of the
   design's exists (O9), thus it is never a Prop and never a truncation.
   LendingLedger and Config come in Part 2 (ledger_source), amendPath in
   Part 3. */
#define DAO_SOURCE_NAME "kernel/dao.lang"

static const char dao_source[] =
  "def Obj : Type 0 := Agent -> Choice\n"
  "def act : G -> Obj -> Obj := fun (g : G) (x : Obj) (a : Agent) => actChoice g (x a)\n"
  "def orbitProjection : Obj -> Orbit := orbitProj\n"
  "def ChoiceRule : Type 0 -> Type 0 -> Type 0 := fun (X : Type 0) (Y : Type 0) => X -> Y\n"
  "def Constitution : Type 0 := Sigma (F : ChoiceRule Obj D) ((g : G) -> (x : Obj) -> Path D (F (act g x)) (F x))\n"
  "def Aggregation : ChoiceRule Obj D -> Type 0 :=\n"
  "  fun (F : ChoiceRule Obj D) => Sigma (L : Orbit -> D) ((x : Obj) -> Path D (L (orbitProj x)) (F x))\n"
  "def Gov : (F : ChoiceRule Obj D) -> Aggregation F -> ChoiceRule Obj D :=\n"
  "  fun (F : ChoiceRule Obj D) (L : Aggregation F) (x : Obj) => witness L (orbitProj x)\n"
  "def IsFixedPointObj : (F : ChoiceRule Obj D) -> Aggregation F -> Type 0 :=\n"
  "  fun (F : ChoiceRule Obj D) (L : Aggregation F) => (x : Obj) -> Path D (Gov F L x) (F x)\n"
  "def IsSelfConstituting : (F : ChoiceRule Obj D) -> Aggregation F -> Type 0 :=\n"
  "  fun (F : ChoiceRule Obj D) (L : Aggregation F) => IsFixedPointObj F L\n";

/* The DAO prelude comes after the last of these definitions. orbitProj is
   in the list, thus the prelude also comes after Orbit. */
static const char *const DAO_AFTER[] = {"Agent", "Choice", "G", "D", "actChoice", "orbitProj"};

/* The ledger as kernel text (lend-lang S7, PLAN section 8, Part 2). With
   2 := Sum Unit Unit, Holding is Sum Position (Sum Position Unit): inl p is
   Active p, inr (inl p) is Forfeited p and inr (inr u) is Empty. Inv b h is
   Unit when the holding agrees with the member bit b (Active at 1,
   Forfeited at 0, Empty at each bit). Else it is a closed path between 0
   and 1, thus isPropInv needs no set-ness of 2. A ledger at the member set
   m is a holding at each agent with its Inv. admit and exit use the same
   stepEquiv, because holdSwap is an involution. LendingLedger sends each
   admit and exit path to ua of its equivalence. swap is a domain
   constructor (REFUSE_NAME), thus the names holdSwap, holdSwapSwap,
   holdSwap01 and holdSwap10. */
#define LEDGER_SOURCE_NAME "kernel/ledger.lang"

static const char ledger_source[] =
  "def Holding : Type 0 := Sum Position (Sum Position Unit)\n"
  "def Inv : Sum Unit Unit -> Holding -> Type 0 :=\n"
  "  fun (b : Sum Unit Unit) (h : Holding) => elim (fun (_ : Holding) => Type 0)\n"
  "    (fun (p : Position) => elim (fun (_ : Sum Unit Unit) => Type 0) (fun (u : Unit) => Path (Sum Unit Unit) (inl unit) (inr unit)) (fun (u : Unit) => Unit) b)\n"
  "    (fun (t : Sum Position Unit) => elim (fun (_ : Sum Position Unit) => Type 0)\n"
  "      (fun (p : Position) => elim (fun (_ : Sum Unit Unit) => Type 0) (fun (u : Unit) => Unit) (fun (u : Unit) => Path (Sum Unit Unit) (inr unit) (inl unit)) b)\n"
  "      (fun (u : Unit) => Unit) t) h\n"
  "def Ledger : (Agent -> Sum Unit Unit) -> Type 0 :=\n"
  "  fun (m : Agent -> Sum Unit Unit) => Sigma (s : Agent -> Holding) ((a : Agent) -> Inv (m a) (s a))\n"
  "def absurd01 : Path (Sum Unit Unit) (inl unit) (inr unit) -> (X : Type 0) -> X :=\n"
  "  fun (e : Path (Sum Unit Unit) (inl unit) (inr unit)) (X : Type 0) =>\n"
  "    subst (Sum Unit Unit) (fun (z : Sum Unit Unit) => elim (fun (_ : Sum Unit Unit) => Type 0) (fun (u : Unit) => Unit) (fun (u : Unit) => X) z) (inl unit) (inr unit) e unit\n"
  "def absurd10 : Path (Sum Unit Unit) (inr unit) (inl unit) -> (X : Type 0) -> X :=\n"
  "  fun (e : Path (Sum Unit Unit) (inr unit) (inl unit)) (X : Type 0) =>\n"
  "    subst (Sum Unit Unit) (fun (z : Sum Unit Unit) => elim (fun (_ : Sum Unit Unit) => Type 0) (fun (u : Unit) => X) (fun (u : Unit) => Unit) z) (inr unit) (inl unit) e unit\n"
  "def isPropUnit : isProp Unit :=\n"
  "  fun (x : Unit) (y : Unit) => elim (fun (x1 : Unit) => Path Unit x1 y) (elim (fun (y1 : Unit) => Path Unit unit y1) refl y) x\n"
  "def isPropPath01 : isProp (Path (Sum Unit Unit) (inl unit) (inr unit)) :=\n"
  "  fun (x : Path (Sum Unit Unit) (inl unit) (inr unit)) (y : Path (Sum Unit Unit) (inl unit) (inr unit)) =>\n"
  "    absurd01 x (Path (Path (Sum Unit Unit) (inl unit) (inr unit)) x y)\n"
  "def isPropPath10 : isProp (Path (Sum Unit Unit) (inr unit) (inl unit)) :=\n"
  "  fun (x : Path (Sum Unit Unit) (inr unit) (inl unit)) (y : Path (Sum Unit Unit) (inr unit) (inl unit)) =>\n"
  "    absurd10 x (Path (Path (Sum Unit Unit) (inr unit) (inl unit)) x y)\n"
  "def isPropInv : (b : Sum Unit Unit) -> (h : Holding) -> isProp (Inv b h) :=\n"
  "  fun (b : Sum Unit Unit) (h : Holding) => elim (fun (h1 : Holding) => isProp (Inv b h1))\n"
  "    (fun (p : Position) => elim (fun (b1 : Sum Unit Unit) => isProp (Inv b1 (inl p))) (fun (u : Unit) => isPropPath01) (fun (u : Unit) => isPropUnit) b)\n"
  "    (fun (t : Sum Position Unit) => elim (fun (t1 : Sum Position Unit) => isProp (Inv b (inr t1)))\n"
  "      (fun (p : Position) => elim (fun (b1 : Sum Unit Unit) => isProp (Inv b1 (inr (inl p)))) (fun (u : Unit) => isPropUnit) (fun (u : Unit) => isPropPath10) b)\n"
  "      (fun (u : Unit) => isPropUnit) t) h\n"
  "def invPathP : (c : Sum Unit Unit) -> (h0 : Holding) -> (h1 : Holding) -> (e : Path Holding h0 h1) -> (v0 : Inv c h0) -> (v1 : Inv c h1) ->\n"
  "  PathP (fun (i : I) => Inv c (e @ i)) v0 v1 :=\n"
  "  fun (c : Sum Unit Unit) (h0 : Holding) (h1 : Holding) (e : Path Holding h0 h1) (v0 : Inv c h0) (v1 : Inv c h1) =>\n"
  "    J Holding h0 (fun (y : Holding) (p : Path Holding h0 y) => (w : Inv c y) -> PathP (fun (i : I) => Inv c (p @ i)) v0 w)\n"
  "      (fun (w : Inv c h0) => isPropInv c h0 v0 w) h1 e v1\n"
  "def ledgerPath : (m : Agent -> Sum Unit Unit) -> (x : Ledger m) -> (y : Ledger m) ->\n"
  "  ((b : Agent) -> Path Holding (witness x b) (witness y b)) -> Path (Ledger m) x y :=\n"
  "  fun (m : Agent -> Sum Unit Unit) (x : Ledger m) (y : Ledger m) (e : (b : Agent) -> Path Holding (witness x b) (witness y b)) =>\n"
  "    fun (i : I) => pack (fun (b : Agent) => e b @ i)\n"
  "      (fun (b : Agent) => invPathP (m b) (witness x b) (witness y b) (e b) (payload x b) (payload y b) @ i)\n"
  "def holdSwap : Holding -> Holding :=\n"
  "  fun (h : Holding) => elim (fun (_ : Holding) => Holding) (fun (p : Position) => inr (inl p))\n"
  "    (fun (t : Sum Position Unit) => elim (fun (_ : Sum Position Unit) => Holding) (fun (p : Position) => inl p) (fun (u : Unit) => inr (inr u)) t) h\n"
  "def holdSwapSwap : (h : Holding) -> Path Holding (holdSwap (holdSwap h)) h :=\n"
  "  fun (h : Holding) => elim (fun (h1 : Holding) => Path Holding (holdSwap (holdSwap h1)) h1) (fun (p : Position) => refl)\n"
  "    (fun (t : Sum Position Unit) => elim (fun (t1 : Sum Position Unit) => Path Holding (holdSwap (holdSwap (inr t1))) (inr t1))\n"
  "      (fun (p : Position) => refl) (fun (u : Unit) => refl) t) h\n"
  "def sel : (X : Type 0) -> (Y : Type 0) -> Sum X Y -> Holding -> Holding :=\n"
  "  fun (X : Type 0) (Y : Type 0) (t : Sum X Y) (h : Holding) => elim (fun (_ : Sum X Y) => Holding) (fun (x : X) => holdSwap h) (fun (y : Y) => h) t\n"
  "def selSel : (X : Type 0) -> (Y : Type 0) -> (t : Sum X Y) -> (h : Holding) -> Path Holding (sel X Y t (sel X Y t h)) h :=\n"
  "  fun (X : Type 0) (Y : Type 0) (t : Sum X Y) (h : Holding) =>\n"
  "    elim (fun (t1 : Sum X Y) => Path Holding (sel X Y t1 (sel X Y t1 h)) h) (fun (x : X) => holdSwapSwap h) (fun (y : Y) => refl) t\n"
  "def holdSwap01 : (h : Holding) -> Inv (inl unit) h -> Inv (inr unit) (holdSwap h) :=\n"
  "  fun (h : Holding) => elim (fun (h1 : Holding) => Inv (inl unit) h1 -> Inv (inr unit) (holdSwap h1))\n"
  "    (fun (p : Position) => fun (v : Path (Sum Unit Unit) (inl unit) (inr unit)) => fun (i : I) => v @ ~i)\n"
  "    (fun (t : Sum Position Unit) => elim (fun (t1 : Sum Position Unit) => Inv (inl unit) (inr t1) -> Inv (inr unit) (holdSwap (inr t1)))\n"
  "      (fun (p : Position) => fun (v : Unit) => v) (fun (u : Unit) => fun (v : Unit) => v) t) h\n"
  "def holdSwap10 : (h : Holding) -> Inv (inr unit) h -> Inv (inl unit) (holdSwap h) :=\n"
  "  fun (h : Holding) => elim (fun (h1 : Holding) => Inv (inr unit) h1 -> Inv (inl unit) (holdSwap h1))\n"
  "    (fun (p : Position) => fun (v : Unit) => v)\n"
  "    (fun (t : Sum Position Unit) => elim (fun (t1 : Sum Position Unit) => Inv (inr unit) (inr t1) -> Inv (inl unit) (holdSwap (inr t1)))\n"
  "      (fun (p : Position) => fun (v : Path (Sum Unit Unit) (inr unit) (inl unit)) => fun (i : I) => v @ ~i) (fun (u : Unit) => fun (v : Unit) => v) t) h\n"
  "def fwdInv : (m : Agent -> Sum Unit Unit) -> (n : Agent -> Sum Unit Unit) -> (a : Agent) -> (c : Sum Unit Unit) -> (d : Sum Unit Unit) ->\n"
  "  Path (Sum Unit Unit) (m a) c -> Path (Sum Unit Unit) (n a) d -> ((h : Holding) -> Inv c h -> Inv d (holdSwap h)) ->\n"
  "  (b : Agent) -> (t : Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) -> (h : Holding) -> Inv (m b) h ->\n"
  "  Inv (n b) (sel (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b)) t h) :=\n"
  "  fun (m : Agent -> Sum Unit Unit) (n : Agent -> Sum Unit Unit) (a : Agent) (c : Sum Unit Unit) (d : Sum Unit Unit)\n"
  "    (p : Path (Sum Unit Unit) (m a) c) (q : Path (Sum Unit Unit) (n a) d) (w : (h : Holding) -> Inv c h -> Inv d (holdSwap h))\n"
  "    (b : Agent) (t : Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) (h : Holding) (v : Inv (m b) h) =>\n"
  "    elim (fun (t1 : Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) => Inv (n b) (sel (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b)) t1 h))\n"
  "      (fun (e : Path Agent b a) => subst (Sum Unit Unit) (fun (z : Sum Unit Unit) => Inv z (holdSwap h)) d (n b)\n"
  "        (fun (i : I) => trans (Sum Unit Unit) (n b) (n a) d (fun (j : I) => n (e @ j)) q @ ~i)\n"
  "        (w h (subst (Sum Unit Unit) (fun (z : Sum Unit Unit) => Inv z h) (m b) c (trans (Sum Unit Unit) (m b) (m a) c (fun (j : I) => m (e @ j)) p) v)))\n"
  "      (fun (e : Path (Sum Unit Unit) (m b) (n b)) => subst (Sum Unit Unit) (fun (z : Sum Unit Unit) => Inv z h) (m b) (n b) e v)\n"
  "      t\n"
  "def bwdInv : (m : Agent -> Sum Unit Unit) -> (n : Agent -> Sum Unit Unit) -> (a : Agent) -> (c : Sum Unit Unit) -> (d : Sum Unit Unit) ->\n"
  "  Path (Sum Unit Unit) (m a) c -> Path (Sum Unit Unit) (n a) d -> ((h : Holding) -> Inv d h -> Inv c (holdSwap h)) ->\n"
  "  (b : Agent) -> (t : Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) -> (h : Holding) -> Inv (n b) h ->\n"
  "  Inv (m b) (sel (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b)) t h) :=\n"
  "  fun (m : Agent -> Sum Unit Unit) (n : Agent -> Sum Unit Unit) (a : Agent) (c : Sum Unit Unit) (d : Sum Unit Unit)\n"
  "    (p : Path (Sum Unit Unit) (m a) c) (q : Path (Sum Unit Unit) (n a) d) (w : (h : Holding) -> Inv d h -> Inv c (holdSwap h))\n"
  "    (b : Agent) (t : Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) (h : Holding) (v : Inv (n b) h) =>\n"
  "    elim (fun (t1 : Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) => Inv (m b) (sel (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b)) t1 h))\n"
  "      (fun (e : Path Agent b a) => subst (Sum Unit Unit) (fun (z : Sum Unit Unit) => Inv z (holdSwap h)) c (m b)\n"
  "        (fun (i : I) => trans (Sum Unit Unit) (m b) (m a) c (fun (j : I) => m (e @ j)) p @ ~i)\n"
  "        (w h (subst (Sum Unit Unit) (fun (z : Sum Unit Unit) => Inv z h) (n b) d (trans (Sum Unit Unit) (n b) (n a) d (fun (j : I) => n (e @ j)) q) v)))\n"
  "      (fun (e : Path (Sum Unit Unit) (m b) (n b)) => subst (Sum Unit Unit) (fun (z : Sum Unit Unit) => Inv z h) (n b) (m b) (fun (i : I) => e @ ~i) v)\n"
  "      t\n"
  "def ledgerFwd : (m : Agent -> Sum Unit Unit) -> (n : Agent -> Sum Unit Unit) -> (a : Agent) -> (c : Sum Unit Unit) -> (d : Sum Unit Unit) ->\n"
  "  Path (Sum Unit Unit) (m a) c -> Path (Sum Unit Unit) (n a) d -> ((h : Holding) -> Inv c h -> Inv d (holdSwap h)) ->\n"
  "  ((b : Agent) -> Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) -> Ledger m -> Ledger n :=\n"
  "  fun (m : Agent -> Sum Unit Unit) (n : Agent -> Sum Unit Unit) (a : Agent) (c : Sum Unit Unit) (d : Sum Unit Unit)\n"
  "    (p : Path (Sum Unit Unit) (m a) c) (q : Path (Sum Unit Unit) (n a) d) (w : (h : Holding) -> Inv c h -> Inv d (holdSwap h))\n"
  "    (r : (b : Agent) -> Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) (x : Ledger m) =>\n"
  "    pack (fun (b : Agent) => sel (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b)) (r b) (witness x b))\n"
  "      (fun (b : Agent) => fwdInv m n a c d p q w b (r b) (witness x b) (payload x b))\n"
  "def ledgerBwd : (m : Agent -> Sum Unit Unit) -> (n : Agent -> Sum Unit Unit) -> (a : Agent) -> (c : Sum Unit Unit) -> (d : Sum Unit Unit) ->\n"
  "  Path (Sum Unit Unit) (m a) c -> Path (Sum Unit Unit) (n a) d -> ((h : Holding) -> Inv d h -> Inv c (holdSwap h)) ->\n"
  "  ((b : Agent) -> Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) -> Ledger n -> Ledger m :=\n"
  "  fun (m : Agent -> Sum Unit Unit) (n : Agent -> Sum Unit Unit) (a : Agent) (c : Sum Unit Unit) (d : Sum Unit Unit)\n"
  "    (p : Path (Sum Unit Unit) (m a) c) (q : Path (Sum Unit Unit) (n a) d) (w : (h : Holding) -> Inv d h -> Inv c (holdSwap h))\n"
  "    (r : (b : Agent) -> Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) (y : Ledger n) =>\n"
  "    pack (fun (b : Agent) => sel (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b)) (r b) (witness y b))\n"
  "      (fun (b : Agent) => bwdInv m n a c d p q w b (r b) (witness y b) (payload y b))\n"
  "def stepEquiv : (m : Agent -> Sum Unit Unit) -> (n : Agent -> Sum Unit Unit) -> (a : Agent) -> (c : Sum Unit Unit) -> (d : Sum Unit Unit) ->\n"
  "  Path (Sum Unit Unit) (m a) c -> Path (Sum Unit Unit) (n a) d -> ((h : Holding) -> Inv c h -> Inv d (holdSwap h)) ->\n"
  "  ((h : Holding) -> Inv d h -> Inv c (holdSwap h)) ->\n"
  "  ((b : Agent) -> Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) -> Equiv (Ledger m) (Ledger n) :=\n"
  "  fun (m : Agent -> Sum Unit Unit) (n : Agent -> Sum Unit Unit) (a : Agent) (c : Sum Unit Unit) (d : Sum Unit Unit)\n"
  "    (p : Path (Sum Unit Unit) (m a) c) (q : Path (Sum Unit Unit) (n a) d) (w : (h : Holding) -> Inv c h -> Inv d (holdSwap h))\n"
  "    (w2 : (h : Holding) -> Inv d h -> Inv c (holdSwap h)) (r : (b : Agent) -> Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) =>\n"
  "    isoToEquiv (Ledger m) (Ledger n) (ledgerFwd m n a c d p q w r) (ledgerBwd m n a c d p q w2 r)\n"
  "      (fun (y : Ledger n) => ledgerPath n (ledgerFwd m n a c d p q w r (ledgerBwd m n a c d p q w2 r y)) y\n"
  "        (fun (b : Agent) => selSel (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b)) (r b) (witness y b)))\n"
  "      (fun (x : Ledger m) => ledgerPath m (ledgerBwd m n a c d p q w2 r (ledgerFwd m n a c d p q w r x)) x\n"
  "        (fun (b : Agent) => selSel (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b)) (r b) (witness x b)))\n"
  "def admitEquiv : (m : Agent -> Sum Unit Unit) -> (n : Agent -> Sum Unit Unit) -> (a : Agent) ->\n"
  "  Path (Sum Unit Unit) (m a) (inl unit) -> Path (Sum Unit Unit) (n a) (inr unit) ->\n"
  "  ((b : Agent) -> Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) -> Equiv (Ledger m) (Ledger n) :=\n"
  "  fun (m : Agent -> Sum Unit Unit) (n : Agent -> Sum Unit Unit) (a : Agent) (p : Path (Sum Unit Unit) (m a) (inl unit))\n"
  "    (q : Path (Sum Unit Unit) (n a) (inr unit)) (r : (b : Agent) -> Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) =>\n"
  "    stepEquiv m n a (inl unit) (inr unit) p q holdSwap01 holdSwap10 r\n"
  "def exitEquiv : (m : Agent -> Sum Unit Unit) -> (n : Agent -> Sum Unit Unit) -> (a : Agent) ->\n"
  "  Path (Sum Unit Unit) (m a) (inr unit) -> Path (Sum Unit Unit) (n a) (inl unit) ->\n"
  "  ((b : Agent) -> Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) -> Equiv (Ledger m) (Ledger n) :=\n"
  "  fun (m : Agent -> Sum Unit Unit) (n : Agent -> Sum Unit Unit) (a : Agent) (p : Path (Sum Unit Unit) (m a) (inr unit))\n"
  "    (q : Path (Sum Unit Unit) (n a) (inl unit)) (r : (b : Agent) -> Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) =>\n"
  "    stepEquiv m n a (inr unit) (inl unit) p q holdSwap10 holdSwap01 r\n"
  "def LendingLedger : Membership -> Type 0 :=\n"
  "  fun (x : Membership) => elim (fun (_ : Membership) => Type 0)\n"
  "    (fun (m : Agent -> Sum Unit Unit) => Ledger m)\n"
  "    (fun (m : Agent -> Sum Unit Unit) (n : Agent -> Sum Unit Unit) (a : Agent) (p : Path (Sum Unit Unit) (m a) (inl unit))\n"
  "      (q : Path (Sum Unit Unit) (n a) (inr unit)) (r : (b : Agent) -> Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) =>\n"
  "      ua (Ledger m) (Ledger n) (admitEquiv m n a p q r))\n"
  "    (fun (m : Agent -> Sum Unit Unit) (n : Agent -> Sum Unit Unit) (a : Agent) (p : Path (Sum Unit Unit) (m a) (inr unit))\n"
  "      (q : Path (Sum Unit Unit) (n a) (inl unit)) (r : (b : Agent) -> Sum (Path Agent b a) (Path (Sum Unit Unit) (m b) (n b))) =>\n"
  "      ua (Ledger m) (Ledger n) (exitEquiv m n a p q r)) x\n"
  "def Config : Type 0 :=\n"
  "  Sigma (M : Membership) (Sigma (F : ChoiceRule Obj D) (Sigma (L : Aggregation F) (Prod (IsSelfConstituting F L) (LendingLedger M))))\n";

/* The ledger comes after the last of these definitions. IsSelfConstituting
   is the last DAO definition, thus the ledger also comes after the DAO
   prelude and after Membership. */
static const char *const LEDGER_AFTER[] = {"Position", "IsSelfConstituting"};

/* Law 2 (S8 Part 2, PLAN section 8): two factorizations L and K of one
   rule F through orbitProj are equal on each orbit. descentUnique is an
   elim on Orbit into fun z => Path D (witness L z) (witness K z). The quot
   case is descentPoint, the quotEq case is a PathP by outcomeSet, and the
   quotSquash case is a square by outcomeSet. outcomeSet is a program slot
   (isSet D), thus the block comes after IsSelfConstituting and
   outcomeSet. R is ORBIT_R, as in the orbit source. */
#define LAW2_SOURCE_NAME "kernel/law2.lang"

static const char law2_source[] =
  "def orbitEq : (a : Obj) -> (b : Obj) -> Sigma (g : G) (Path (Agent -> Choice) (fun (t : Agent) => actChoice g (b t)) a) -> Path Orbit (orbitProj a) (orbitProj b) :=\n"
  "  fun (a : Obj) (b : Obj) (r : Sigma (g : G) (Path (Agent -> Choice) (fun (t : Agent) => actChoice g (b t)) a)) => fun (i : I) => quotEq (Agent -> Choice) " ORBIT_R " a b r i\n"
  "def orbitSet : isSet Orbit :=\n"
  "  fun (x : Orbit) (y : Orbit) (p : Path Orbit x y) (q : Path Orbit x y) => fun (i : I) => fun (j : I) => quotSquash (Agent -> Choice) " ORBIT_R " x y p q i j\n"
  "def descentPoint : (F : ChoiceRule Obj D) -> (L : Aggregation F) -> (K : Aggregation F) -> (x : Obj) -> Path D (witness L (orbitProj x)) (witness K (orbitProj x)) :=\n"
  "  fun (F : ChoiceRule Obj D) (L : Aggregation F) (K : Aggregation F) (x : Obj) =>\n"
  "    fun (i : I) => hcomp D (~i \\/ i) (fun (k : I) => [ ~i -> witness L (orbitProj x)"
  ", i -> sym (payload K x) @ k ]) (payload L x @ i)\n"
  "def descentUnique : (F : ChoiceRule Obj D) -> (L : Aggregation F) -> (K : Aggregation F) -> (z : Orbit) -> Path D (witness L z) (witness K z) :=\n"
  "  fun (F : ChoiceRule Obj D) (L : Aggregation F) (K : Aggregation F) (z : Orbit) =>\n"
  "    elim (fun (w : Orbit) => Path D (witness L w) (witness K w))\n"
  "      (fun (x : Obj) => descentPoint F L K x)\n"
  "      (fun (a : Obj) (b : Obj) (r : Sigma (g : G) (Path (Agent -> Choice) (fun (t : Agent) => actChoice g (b t)) a)) => fun (i : I) =>\n"
  "        outcomeSet (witness L (orbitEq a b r @ i)) (witness K (orbitEq a b r @ i))\n"
  "          (transp (fun (j : I) => Path D (witness L (orbitEq a b r @ (i /\\ j))) (witness K (orbitEq a b r @ (i /\\ j)))) ~i (descentPoint F L K a))\n"
  "          (transp (fun (j : I) => Path D (witness L (orbitEq a b r @ (i \\/ ~j))) (witness K (orbitEq a b r @ (i \\/ ~j)))) i (descentPoint F L K b)) @ i)\n"
  "      (fun (x : Orbit) (ihx : Path D (witness L x) (witness K x)) (y : Orbit) (ihy : Path D (witness L y) (witness K y))\n"
  "        (p : Path Orbit x y) (ihp : PathP (fun (j : I) => Path D (witness L (p @ j)) (witness K (p @ j))) ihx ihy)\n"
  "        (q : Path Orbit x y) (ihq : PathP (fun (j : I) => Path D (witness L (q @ j)) (witness K (q @ j))) ihx ihy) =>\n"
  "        fun (i : I) => fun (j : I) => hcomp (Path D (witness L (orbitSet x y p q @ i @ j)) (witness K (orbitSet x y p q @ i @ j))) (~i \\/ i \\/ ~j \\/ j) (fun (k : I) => [ ~i -> outcomeSet (witness L (orbitSet x y p q @ i @ j)) (witness K (orbitSet x y p q @ i @ j)) (transp (fun (k : I) => Path D (witness L (orbitSet x y p q @ (i /\\ k) @ (j /\\ k))) (witness K (orbitSet x y p q @ (i /\\ k) @ (j /\\ k)))) i0 ihx) (ihp @ j) @ k"
  ", i -> outcomeSet (witness L (orbitSet x y p q @ i @ j)) (witness K (orbitSet x y p q @ i @ j)) (transp (fun (k : I) => Path D (witness L (orbitSet x y p q @ (i /\\ k) @ (j /\\ k))) (witness K (orbitSet x y p q @ (i /\\ k) @ (j /\\ k)))) i0 ihx) (ihq @ j) @ k"
  ", ~j -> outcomeSet (witness L (orbitSet x y p q @ i @ j)) (witness K (orbitSet x y p q @ i @ j)) (transp (fun (k : I) => Path D (witness L (orbitSet x y p q @ (i /\\ k) @ (j /\\ k))) (witness K (orbitSet x y p q @ (i /\\ k) @ (j /\\ k)))) i0 ihx) ihx @ k"
  ", j -> outcomeSet (witness L (orbitSet x y p q @ i @ j)) (witness K (orbitSet x y p q @ i @ j)) (transp (fun (k : I) => Path D (witness L (orbitSet x y p q @ (i /\\ k) @ (j /\\ k))) (witness K (orbitSet x y p q @ (i /\\ k) @ (j /\\ k)))) i0 ihx) ihy @ k ]) (transp (fun (k : I) => Path D (witness L (orbitSet x y p q @ (i /\\ k) @ (j /\\ k))) (witness K (orbitSet x y p q @ (i /\\ k) @ (j /\\ k)))) i0 ihx))\n"
  "      z\n";

static const char *const LAW2_AFTER[] = {"IsSelfConstituting", "outcomeSet"};

/* Put the declarations of TEXT directly after the declaration AT, with the
   trusted origin ORIGIN_DOMAIN. Check order is declaration order, so a
   refused declaration at or before AT stops before them. */
static int insert_after(Arena *arena, size_t at, const char *source, const char *text, size_t len, DeclList *out, Diag *diag) {
  DeclList kernel = {NULL, 0, 0};
  Decl *items;
  if (!load_one(arena, source, ORIGIN_DOMAIN, text, len, &kernel, diag))
    return 0;
  items = arena_alloc(arena, (out->count + kernel.count) * sizeof *items);
  if (items == NULL)
    return 0;
  memcpy(items, out->items, (at + 1u) * sizeof *items);
  memcpy(items + at + 1u, kernel.items, kernel.count * sizeof *items);
  memcpy(items + at + 1u + kernel.count, out->items + at + 1u, (out->count - at - 1u) * sizeof *items);
  out->items = items;
  out->count += kernel.count;
  out->cap = out->count;
  return 1;
}

/* The position of the first `def NAME` from FIRST on, or the declaration
   count when there is none. */
static size_t def_at(const DeclList *out, size_t first, const char *name) {
  size_t at;
  for (at = first; at < out->count && (out->items[at].kind != DECL_DEF || strcmp(out->items[at].name, name) != 0); at++)
    ;
  return at;
}

/* When the program from FIRST on defines NAME, put TEXT directly after it. */
static int splice_after(Arena *arena, size_t first, const char *name, const char *source, const char *text, size_t len, DeclList *out, Diag *diag) {
  size_t at = def_at(out, first, name);
  return at == out->count || insert_after(arena, at, source, text, len, out, diag);
}

/* When the declarations from FIRST on define each of the COUNT NAMES, put
   TEXT directly after the last of them. */
static int splice_after_all(Arena *arena, size_t first, const char *const *names, size_t count, const char *source, const char *text, size_t len, DeclList *out, Diag *diag) {
  size_t last = first;
  size_t i;
  for (i = 0; i < count; i++) {
    size_t at = def_at(out, first, names[i]);
    if (at == out->count)
      return 1;
    if (at > last)
      last = at;
  }
  return insert_after(arena, last, source, text, len, out, diag);
}

/* Membership directly after the program `def Agent` (USER ruling S6),
   Orbit and orbitProj directly after the program `def actChoice`, the DAO
   prelude after the last of its slots and Orbit (S7), the ledger after
   Position and the DAO prelude (S7 Part 2), and law 2 after
   IsSelfConstituting and outcomeSet (S8 Part 2). */
static int splice_kernel(Arena *arena, size_t first, DeclList *out, Diag *diag) {
  return splice_after(arena, first, "Agent", MEMBERSHIP_SOURCE_NAME, membership_source, sizeof membership_source - 1u, out, diag)
      && splice_after(arena, first, "actChoice", ORBIT_SOURCE_NAME, orbit_source, sizeof orbit_source - 1u, out, diag)
      && splice_after_all(arena, first, DAO_AFTER, sizeof DAO_AFTER / sizeof DAO_AFTER[0], DAO_SOURCE_NAME, dao_source, sizeof dao_source - 1u, out, diag)
      && splice_after_all(arena, first, LEDGER_AFTER, sizeof LEDGER_AFTER / sizeof LEDGER_AFTER[0], LEDGER_SOURCE_NAME, ledger_source, sizeof ledger_source - 1u, out, diag)
      && splice_after_all(arena, first, LAW2_AFTER, sizeof LAW2_AFTER / sizeof LAW2_AFTER[0], LAW2_SOURCE_NAME, law2_source, sizeof law2_source - 1u, out, diag);
}

int front_load(Arena *arena, const char *source, const char *text, size_t len, DeclList *out, Diag *diag) {
  size_t first;
  out->items = NULL;
  out->count = 0;
  out->cap = 0;
  if (!load_one(arena, "domain/domain.lang", ORIGIN_DOMAIN, (const char *)domain_source, domain_source_len, out, diag)
      || !load_one(arena, CONTRACT_SOURCE_NAME, ORIGIN_KIT, contract_source, contract_source_len, out, diag))
    return 0;
  first = out->count;
  return load_one(arena, source, ORIGIN_PROGRAM, text, len, out, diag)
      && splice_kernel(arena, first, out, diag);
}

int front_load_domain(Arena *arena, const char *source, const char *domain, const char *text, size_t len, DeclList *out, Diag *diag) {
  size_t first;
  out->items = NULL;
  out->count = 0;
  out->cap = 0;
  if (!load_one(arena, "domain/domain.lang", ORIGIN_DOMAIN, (const char *)domain_source, domain_source_len, out, diag)
      || !load_one(arena, CONTRACT_SOURCE_NAME, ORIGIN_KIT, contract_source, contract_source_len, out, diag)
      || !load_one(arena, source, ORIGIN_DOMAIN, domain, strlen(domain), out, diag))
    return 0;
  first = out->count;
  return load_one(arena, source, ORIGIN_PROGRAM, text, len, out, diag)
      && splice_kernel(arena, first, out, diag);
}
