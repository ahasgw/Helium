#include <Helium/smirks.h>
#include <Helium/hemol.h>
#include <Helium/smiles.h>

#include "test.h"

using namespace Helium;

void test_init()
{
  Smirks smirks;

  // valid
  ASSERT(smirks.init("CC>>CC"));
  ASSERT(smirks.init("[C:1]C>>[C:1]C"));
  ASSERT(smirks.init("[C:1][C:2]>>[C:1][O:2]"));

  //
  // invalid
  //

  // no >>
  ASSERT(!smirks.init("CCCC"));
  COMPARE(SmirksError::NoReaction, smirks.error().type());

  // invalid reactant smarts
  ASSERT(!smirks.init("gsgsds>>CC"));
  COMPARE(SmirksError::ReactantSmarts, smirks.error().type());
  // invalid product smarts
  ASSERT(!smirks.init("CC>>gsgss"));
  COMPARE(SmirksError::ProductSmarts, smirks.error().type());

  // invalid atom classes
  ASSERT(!smirks.init("[C:1][C:2]>>[C:1]C"));
  COMPARE(SmirksError::AtomClassPairWise, smirks.error().type());
  ASSERT(!smirks.init("[C:1]C>>[C:1][C:2]"));
  COMPARE(SmirksError::AtomClassPairWise, smirks.error().type());
  ASSERT(!smirks.init("[C:1][C:1]>>[C:1][C:2]"));
  COMPARE(SmirksError::AtomClassPairWise, smirks.error().type());
  ASSERT(!smirks.init("[C:1][C:2]>>[C:1][C:3]"));
  COMPARE(SmirksError::AtomClassPairWise, smirks.error().type());

  // product contains OR
  ASSERT(!smirks.init("[C:1]>>[C,N:1]"));
  COMPARE(SmirksError::ProductContainsOr, smirks.error().type());
  // product contains NOT
  ASSERT(!smirks.init("[C:1]>>[!C:1]"));
  COMPARE(SmirksError::ProductContainsNot, smirks.error().type());

  // invalid product bond
  ASSERT(!smirks.init("CC>>C-,=C"));
  COMPARE(SmirksError::InvalidProductBond, smirks.error().type());
}

void test_simple_atoms()
{
  // C -> O
  {
    HeMol mol = hemol_from_smiles("C");
    Smirks smirks;
    smirks.init("[C:1]>>[O:1]");
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(8, get_element(mol, get_atom(mol, 0)));
  }
  // C -> O-
  {
    HeMol mol = hemol_from_smiles("C");
    Smirks smirks;
    smirks.init("[C:1]>>[O-:1]");
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(8, get_element(mol, get_atom(mol, 0)));
    COMPARE(-1, get_charge(mol, get_atom(mol, 0)));
  }
  // C -> 13C
  {
    HeMol mol = hemol_from_smiles("C");
    Smirks smirks;
    smirks.init("[C:1]>>[13C:1]");
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(6, get_element(mol, get_atom(mol, 0)));
    COMPARE(13, get_mass(mol, get_atom(mol, 0)));
  }
  // C -> N+
  {
    HeMol mol = hemol_from_smiles("C");
    Smirks smirks;
    smirks.init("[C:1]>>[#7+:1]");
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(7, get_element(mol, get_atom(mol, 0)));
    COMPARE(1, get_charge(mol, get_atom(mol, 0)));
  }
  // Ch4 -> Ch2
  {
    HeMol mol = hemol_from_smiles("C");
    Smirks smirks;
    smirks.setFixHydrogens(false);
    smirks.init("[C:1]>>[Ch2:1]");
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(6, get_element(mol, get_atom(mol, 0)));
    COMPARE(2, num_hydrogens(mol, get_atom(mol, 0)));
  }
  // CH4 -> CH3
  {
    HeMol mol = hemol_from_smiles("C");
    Smirks smirks;
    smirks.setFixHydrogens(false);
    ASSERT(smirks.init("[C:1]>>[CH3:1]"));
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(6, get_element(mol, get_atom(mol, 0)));
    COMPARE(3, num_hydrogens(mol, get_atom(mol, 0)));

    mol = hemol_from_smiles("C[H]");
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(6, get_element(mol, get_atom(mol, 0)));
    COMPARE(2, num_hydrogens(mol, get_atom(mol, 0)));
  }
}

void test_simple_bond_change()
{
  // C-C -> C=C
  {
    HeMol mol = hemol_from_smiles("CC");
    Smirks smirks;
    ASSERT(smirks.init("[C:1][C:2]>>[C:1]=[C:2]"));
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(2, get_order(mol, get_bond(mol, 0)));
  }
  // C-C -> C#C
  {
    HeMol mol = hemol_from_smiles("CC");
    Smirks smirks;
    ASSERT(smirks.init("[C:1][C:2]>>[C:1]#[C:2]"));
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(3, get_order(mol, get_bond(mol, 0)));
  }
  // C-C -> C:C
  {
    HeMol mol = hemol_from_smiles("CC");
    Smirks smirks;
    ASSERT(smirks.init("[C:1][C:2]>>[C:1]:[C:2]"));
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(5, get_order(mol, get_bond(mol, 0)));
    COMPARE(true, is_aromatic(mol, get_bond(mol, 0)));
  }
  // c:c -> C-C
  {
    HeMol mol = hemol_from_smiles("cc");
    Smirks smirks;
    ASSERT(smirks.init("[c:1][c:2]>>[C:1]-[C:2]"));
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(1, get_order(mol, get_bond(mol, 0)));
    COMPARE(false, is_aromatic(mol, get_bond(mol, 0)));
  }
  // CC-O -> CC=O
  {
    HeMol mol = hemol_from_smiles("CCO");
    Smirks smirks;
    ASSERT(smirks.init("[C:1][C:2][O:3]>>[C:1][C:2]=[O:3]"));
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(1, get_order(mol, get_bond(mol, 0)));
    COMPARE(2, get_order(mol, get_bond(mol, 1)));
  }
}

void test_simple_bond_added()
{
  // C.C -> C-C
  {
    HeMol mol = hemol_from_smiles("C.C");
    Smirks smirks;
    ASSERT(smirks.init("[C:1].[C:2]>>[C:1][C:2]"));
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(1, num_bonds(mol));
    COMPARE(1, get_order(mol, get_bond(mol, 0)));
  }
  // CC.C -> CC=C
  {
    HeMol mol = hemol_from_smiles("CC.C");
    Smirks smirks;
    ASSERT(smirks.init("[C:3].[C:1][C:2]>>[C:1][C:2]=[C:3]"));
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(2, num_bonds(mol));
    COMPARE(1, get_order(mol, get_bond(mol, 0)));
    COMPARE(2, get_order(mol, get_bond(mol, 1)));
  }
  // C.C.C -> C-C-C
  {
    HeMol mol = hemol_from_smiles("C.C.C");
    Smirks smirks;
    ASSERT(smirks.init("[C:3].[C:1].[C:2]>>[C:1][C:2][C:3]"));
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(2, num_bonds(mol));
    COMPARE(1, get_order(mol, get_bond(mol, 0)));
    COMPARE(1, get_order(mol, get_bond(mol, 1)));
  }
  // C.O.C.O -> C-O.C-O
  {
    HeMol mol = hemol_from_smiles("O.O.C.C");
    Smirks smirks;
    ASSERT(smirks.init("[C:1].[C:2].[O:3].[O:4]>>[C:1][O:3].[C:2][O:4]"));
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(2, num_bonds(mol));
    COMPARE(1, get_order(mol, get_bond(mol, 0)));
    ASSERT(is_oxygen(mol, get_source(mol, get_bond(mol, 0))) || is_oxygen(mol, get_target(mol, get_bond(mol, 0))));
    ASSERT(is_carbon(mol, get_source(mol, get_bond(mol, 0))) || is_carbon(mol, get_target(mol, get_bond(mol, 0))));
    COMPARE(1, get_order(mol, get_bond(mol, 1)));
    ASSERT(is_oxygen(mol, get_source(mol, get_bond(mol, 1))) || is_oxygen(mol, get_target(mol, get_bond(mol, 1))));
    ASSERT(is_carbon(mol, get_source(mol, get_bond(mol, 1))) || is_carbon(mol, get_target(mol, get_bond(mol, 1))));
  }
}

void test_simple_bond_removed()
{
  // C-C -> C.C
  {
    HeMol mol = hemol_from_smiles("CC");
    Smirks smirks;
    ASSERT(smirks.init("[C:1][C:2]>>[C:1].[C:2]"));
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(0, num_bonds(mol));
  }
  // C-C-C -> C.C.C
  {
    HeMol mol = hemol_from_smiles("CCC");
    Smirks smirks;
    ASSERT(smirks.init("[C:1][C:2][C:3]>>[C:1].[C:2].[C:3]"));
    smirks.apply(mol, RingSet<HeMol>(mol));
    COMPARE(0, num_bonds(mol));
  }
}

bool smarts_match(const HeMol &mol, const std::string &smarts)
{
  Smarts s;
  ASSERT(s.init(smarts));
  return s.search(mol, RingSet<HeMol>(mol));
}

bool smarts_match(const std::string &smiles, const std::string &smarts)
{
  Smarts s;
  ASSERT(s.init(smarts));
  HeMol mol = hemol_from_smiles(smiles);
  return s.search(mol, RingSet<HeMol>(mol));
}

void test_smirks(const std::string &smirks, const std::string smiles,
    const std::string &smarts1 = std::string(),
    const std::string &smarts2 = std::string(),
    const std::string &smarts3 = std::string(),
    const std::string &smarts4 = std::string(),
    const std::string &smarts5 = std::string())
{
  std::cout << "Tesing: " << smirks << " on " << smiles << std::endl;
  HeMol mol = hemol_from_smiles(smiles);
  Smirks s;
  if (!s.init(smirks)) {
    std::cout << "Error: " << s.error().what() << std::endl;
    return;
  }

  if (s.requiresExplicitHydrogens())
    make_hydrogens_explicit(mol);

  if (!s.apply(mol, RingSet<HeMol>(mol)))
    std::cout << "    reactant SMARTS did not match molecule!" << std::endl;

  if (s.requiresExplicitHydrogens())
    make_hydrogens_implicit(mol);

  std::cout << "    transformed molecule: " << SMILES.write(mol) << std::endl;

  if (smarts1.size() && !smarts_match(mol, smarts1)) {
    std::cout << "    " << smarts1 << " not found!" << std::endl;
    ASSERT(0);
  }
  if (smarts2.size() && !smarts_match(mol, smarts2)) {
    std::cout << "    " << smarts2 << " not found!" << std::endl;
    ASSERT(0);
  }
  if (smarts3.size() && !smarts_match(mol, smarts3)) {
    std::cout << "    " << smarts3 << " not found!" << std::endl;
    ASSERT(0);
  }
  if (smarts4.size() && !smarts_match(mol, smarts4)) {
    std::cout << "    " << smarts4 << " not found!" << std::endl;
    ASSERT(0);
  }
  if (smarts5.size() && !smarts_match(mol, smarts5)) {
    std::cout << "    " << smarts5 << " not found!" << std::endl;
    ASSERT(0);
  }
}

void test_complex()
{
  // pentavalent nitro to charged nitro
  test_smirks("[*:1][N:2](=[O:3])=[O:4]>>[*:1][N+:2](=[O:3])[O-:4]",
      "CN(=O)=O", "CN(=O)-[O-]");
  test_smirks("[*:1][N:2](=[O:3])=[O:4]>>[*:1][N+:2](=[O:3])[O-:4]",
      "CCN(=O)=O", "CN(=O)-[O-]");
  test_smirks("[*:1][N:2](=[O:3])=[O:4]>>[*:1][N+:2](=[O:3])[O-:4]",
      "CCCN(=O)=O", "CN(=O)-[O-]");

  // test adding/ removing atoms w/o atom class
  test_smirks("CC[C:1]>>[N:1]", "CCC", "[ND0]");
  test_smirks("CC[C:1]>>[N:1].OO", "CCC", "N.OO");
  test_smirks("[C:1]>>[N:1].O", "C.C", "N.O.N.O");
  test_smirks("[C:1]>>[N:1]O", "C.C", "NO.NO");
  test_smirks("[C:1]>>[C:1]1CCC1", "C.C", "C1CCC1.C1CCC1");


  test_smirks("[C:1]=[C:2][C:3]=[C:4].[C:5]=[C:6]>>[C:1]1[C:2]=[C:3][C:4][C:5][C:6]1",
      "C=CC=C.C=C", "C1=CCCCC1");
  test_smirks("[*;Br,I:3][C:2].[*+0;n,N,S,O:1]>>[*-:3].[*+:1][C:2]", "CBr.CCOCC", "CC[O+](C)CC.[Br-]");
  test_smirks("Cl[C:1]=[O:2]>>N[C:1]=[O:2]", "CCC(=O)Cl", "CCC(=O)N");
  test_smirks("[C:1]=O>>[C:1]1OCCO1", "C1CCCCC1=O", "C1CCCCC12OCCO2");
  test_smirks("[C:1]=[O:2]>>[C:1][O:2]", "O=CCCC(C)C=O", "OCCCC(C)CO");
  test_smirks("[C:2]1[C:3][CH:4]=[N:5][C:6]1.[N+:7]#[C-:8].[C:10](=[O:11])[O:12][H:88]>>[H:88][N:7][C:8](=[O:12])[CH:4]([C:3][C:2]1)[N:5]([C:10]=[O:11])[C:6]1",
      "C1CC=NC1.[C-]#[N+]CCC.c1cc(Cl)ccc1C(=O)O", "CCCNC(=O)C1CCCN1C(=O)c2ccc(Cl)cc2");
  test_smirks("[C:1](=[O:2])[Cl:3].[H:99][N:4]([H:100])[C:5]>>[C:1](=[O:2])[N:4]([H:100])[C:5].[Cl:3][H:99]",
      "CCCCN.CCCC(=O)Cl", "CCCCNC(=O)CCC.Cl");
  test_smirks("[C:1](=[O:2])[Cl:3].[H:99][N:4]([H:100])[C:0]>>[C:1](=[O:2])[N:4]([H:100])[C:0].[Cl:3][H:99]",
      "CCCCN.CCCC(=O)Cl", "CCCCNC(=O)CCC.Cl");
  test_smirks("[*:1][C:2]([H:3])([O:4][H:5])[C:6]([H:7])([O:8][H:9])[*:10]>>[*:1][C:2](=[O:4])[C:6](=[O:8])[*:10].[H:3][H:5].[H:7][H:9]",
      "C1CC(O)C(O)CC1", "O=C1CCCCC1=O");
  test_smirks("[*:1][CH:2]([OH:3])[*:4].[I:5][SiH2:6][I:7]>>[*:1][CH:2]([I:5])[*:4].[OH:3][SiH2:6][I:7]",
      "I[SiH2]I.CCCCC(O)CCC", "CCCCC(I)CCC.O[SiH2]I");

  test_smirks("[H][N:4]([H])[C:5].[C:1](=[O:2])[Cl]>>[C:1](=[O:2])[N:4]([H])[C:5].[Cl][H]",
      "CCCN.CCC(=O)Cl", "CCCNC(=O)CC.Cl");
  test_smirks("[H][N:4]([H])[C:5].[C:1](=[O:2])[Cl]>>[C:1](=[O:2])[N:4]([H])[C:5].[Cl][H]",
      "CCCN.c1ccccc1CC(=O)Cl", "CCCNC(=O)Cc1ccccc1.Cl");
  test_smirks("[H][N:4]([H])[C:5].[C:1](=[O:2])[Cl]>>[C:1](=[O:2])[N:4]([H])[C:5].[Cl][H]",
      "CCN.CCC(=O)Cl", "CCNC(=O)CC.Cl");
  test_smirks("[H][N:4]([H])[C:5].[C:1](=[O:2])[Cl]>>[C:1](=[O:2])[N:4]([H])[C:5].[Cl][H]",
      "CCN.c1ccccc1CC(=O)Cl", "CCNC(=O)Cc1ccccc1.Cl");

  //test_smirks("", "", "");
}

int main()
{
  Smirks smirks;
  smirks.init("[C:1][C:2]>>[C:1][O:2]");

  test_init();

  test_simple_atoms();
  test_simple_bond_change();
  test_simple_bond_added();
  test_simple_bond_removed();
  test_complex();
}
