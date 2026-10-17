# lend-lang domain design

> Paste the domain design below the line, verbatim. Do not edit it.

This file is the only source of the core types and the core operations of
lend-lang. `SPEC.md` sections 5 and 6 cite its sections. Do not add a core
type or a core operation that this file does not give.

When the design changes, paste the new version. Then record the change as an
open item in `SPEC.md` section 9.

---

## The request (verbatim, 2026-10-17)

I want to write a new programming language with the following features:

For type constructors and higher kinded types, admit products, coproducts, monads (map/bind), algebras/coalgebras (fold/unfold), dependent types, and quotients/paths/loops/HITs/QITs/HIITs/QIITs as in Cubical TT.

Only admit the base types and operations from the following denotational design:

[Editorial note: the pasted design is the next section.]

Proof irrelevance is prohibited.

The host language shall be TinyCC, and the compilation target should be EVM deployable bytecode to any chain, such as Telcoin Network, mainnet Ethereum, etc.

## The design (verbatim, 2026-10-17)

**Denotational Design (Conal Elliott) of a Zero-Knowledge Lending Smart Contract Governed by a Zero-Knowledge Self-Referential DAO**

Following Conal Elliott’s methodology, we specify the mathematical *meaning* (denotation) of the system first in Cubical Type Theory, then derive the operations so that the meaning function \(\llbracket\cdot\rrbracket\) is a homomorphism (the instance’s meaning is the meaning’s instance). Correctness of any representation (circuit, contract, or implementation) is exact agreement with this denotation. We work in Cubical TT (Agda-style notation) so that equalities are paths with computational content, univalence/Glue is available, and higher structure (fixed points, quotients, private/public views) is native.

The design reuses the semantic core of the referenced repository:

\[
\llbracket\mathrm{DAO}\rrbracket = \mathrm{Aggregation}\,\mathrm{act}\,F = \mathrm{LeftKanExtension}\,(\mathrm{orbitProjection}\,\mathrm{act})\,F
\]

with the governance endo-operation

\[
\mathrm{Gov}\,(L : \mathrm{Aggregation}\,\mathrm{act}\,F) : \mathrm{ChoiceRule}\,\mathrm{Obj}\,D := \mathrm{orbitProjection}\,\mathrm{act} \mathbin{\circ\!\!\!\circ} L.\mathrm{functor}
\]

and self-constitution as the fixed-point predicate \(\mathrm{IsSelfConstituting}\,F := \exists L,\;\forall X,\;(\mathrm{Gov}\,L).\mathrm{obj}\,X = F.\mathrm{obj}\,X\). Membership is the domain of the action (coordinates of configurations acted on by the symmetry group \(G\)); anonymity is the orbit quotient. Each amendment is forced to be the homomorphism \(\llbracket\mathrm{amend}\rrbracket = \mathrm{Gov}\).

### Semantic Domain

The whole system denotes a configuration in the following Σ-type (the single semantic domain into which every construct is interpreted):

```agda
Config : Type
Config = Σ[ M : Membership ]
         Σ[ F : ChoiceRule (Obj M) D ]
         Σ[ L : Aggregation (act M) F ]
         (IsSelfConstituting L × LendingLedger M)
```

- \(\mathrm{Membership}\) is a higher inductive type (or a finite set with path constructors) whose points are member sets and whose path constructors are exactly the admittance and exit operations. Consequently every path in \(\mathrm{Membership}\) is a membership change.
- \(\mathrm{Obj}\,M\) and \(\mathrm{act}\,M\) are the configuration category and \(G\)-action determined by the current membership (so the orbit quotient, and therefore anonymity/ZK of voting, depends on who is a member).
- \(\mathrm{LendingLedger}\,M\) is the type of private loan positions indexed by the current members (commitments, collateral, rates, etc.). The public ledger is the image under a commitment map; the private data never appear in the public face.

The meaning function of a concrete system representation is the total map into this \(\mathrm{Config}\).

### Amendments (self-reference + membership admittance/exit)

An amendment is an operation whose denotation is forced by the homomorphism principle:

\[
\llbracket\mathrm{amend}\rrbracket\,(M,F,L,\mathrm{sc},\mathrm{LS}) = (M',\;\mathrm{Gov}\,L,\;L',\;\mathrm{sc}',\;\mathrm{LS}')
\]

where:

- the membership component \(M \equiv M'\) is a path whose constructors are the proposed admits and exits (so every amendment *contains* the membership change as a sub-path);
- the constitution is replaced by \(\mathrm{Gov}\,L\) (exactly the original \(\llbracket\mathrm{amend}\rrbracket = \mathrm{Gov}\));
- \(L'\) is the transport of the aggregation along the path of constitutions and memberships;
- \(\mathrm{sc}'\) is the transported fixed-point witness, expressed by a dependent path

```agda
sc' : PathP (λ i → IsFixedPointObj (transported L i)) sc sc-new
```

so the self-constitution property is preserved (or re-established) computationally.

In Cubical notation the whole amendment is a single path

```agda
amendPath : (admits exits : List Agent) (proposal : …) 
          → Path Config c₀ c₁
```

whose \(i=0\) face is the old configuration, whose \(i=1\) face is the new one, and whose interior is filled by the Kan operations so that the constitution face is definitionally \(\mathrm{Gov}\) and the membership face is the HIT path generated by the admits/exits. Because the path is a homomorphism for the Aggregation/Gov structure, the three UAT regimes (Arrow-Impossibility, Arrow-Debreu, Schelling-Ising) are inherited automatically; the fixed-point predicate remains single-valued exactly when the original design says it is.

### Zero-Knowledge Lending Ledger

The lending state is private. A public commitment \(C\) and a private position \(P\) are related by an equivalence (the ZK circuit). The Glue type packages them:

```agda
PrivateLoan : Commitment → Type
PrivateLoan c = Glue c (λ { (φ = i1) → (P , zkEquiv) })
```

where \(\mathrm{zkEquiv} : P \simeq c\) is the computational content of the zero-knowledge proof (circuit satisfaction). A lending step (borrow, repay, liquidate, …) denotes a path in the ledger that is valid precisely when it extends to a path in the Glue type:

```agda
⟦borrow⟧ : (amount : …) (π : ZKProof) 
         → Path (LendingLedger M) LS (LS' amount)
```

The public face of the path only updates the commitment; the private witness is recovered by \(\mathrm{unglue}\). All rate, collateral-ratio and membership-access invariants are enforced by the boundary conditions of the path (they must hold at both endpoints and be preserved by the Kan filling). Because membership is part of the type index, only current members can form a well-typed loan path; an exit that occurs in the same amendment path automatically invalidates subsequent private loans for the exiting member by transport.

### Homomorphism Laws (laws for free)

By construction of the denotation:

- \(\llbracket\mathrm{amend}\rrbracket = \mathrm{Gov}\) (definitional, as in the original design);
- anonymity is the theorem that the aggregation factors through the orbit projection (inherited from UAT);
- self-constitution is a fixed point of the transported \(\mathrm{Gov}\), expressed as a \(\mathrm{PathP}\);
- every lending transition is a homomorphism of the commitment monoid (or whatever algebraic structure the ledger carries) on the public face, while the private face is related by the Glue equivalence.

Thus any implementation whose operational behaviour agrees with these path-equalities is correct by the denotational criterion; abstraction leaks are impossible because there is no residual “how” outside the meaning.

This yields a precise, compositional, and machine-checkable specification in Cubical TT of a ZK lending contract whose governance, membership changes, and self-amendment are all internal to the same denotational domain as the self-referential DAO of the referenced repository.
