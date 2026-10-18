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

/* When the program from FIRST on defines NAME, put the declarations of
   TEXT directly after it, with the trusted origin ORIGIN_DOMAIN. Check
   order is declaration order, so a refused NAME stops before them. */
static int splice_after(Arena *arena, size_t first, const char *name, const char *source, const char *text, size_t len, DeclList *out, Diag *diag) {
  DeclList orbit = {NULL, 0, 0};
  Decl *items;
  size_t at;
  for (at = first; at < out->count && (out->items[at].kind != DECL_DEF || strcmp(out->items[at].name, name) != 0); at++)
    ;
  if (at == out->count)
    return 1;
  if (!load_one(arena, source, ORIGIN_DOMAIN, text, len, &orbit, diag))
    return 0;
  items = arena_alloc(arena, (out->count + orbit.count) * sizeof *items);
  if (items == NULL)
    return 0;
  memcpy(items, out->items, (at + 1u) * sizeof *items);
  memcpy(items + at + 1u, orbit.items, orbit.count * sizeof *items);
  memcpy(items + at + 1u + orbit.count, out->items + at + 1u, (out->count - at - 1u) * sizeof *items);
  out->items = items;
  out->count += orbit.count;
  out->cap = out->count;
  return 1;
}

/* Membership directly after the program `def Agent` (USER ruling S6), and
   Orbit and orbitProj directly after the program `def actChoice`. */
static int orbit_after_act_choice(Arena *arena, size_t first, DeclList *out, Diag *diag) {
  return splice_after(arena, first, "Agent", MEMBERSHIP_SOURCE_NAME, membership_source, sizeof membership_source - 1u, out, diag)
      && splice_after(arena, first, "actChoice", ORBIT_SOURCE_NAME, orbit_source, sizeof orbit_source - 1u, out, diag);
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
      && orbit_after_act_choice(arena, first, out, diag);
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
      && orbit_after_act_choice(arena, first, out, diag);
}
