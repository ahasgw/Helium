#include <Helium/algorithms/canonical.h>
#include <Helium/fileio/moleculefile.h>
#include <Helium/algorithms/extendedconnectivities.h>
#include <Helium/algorithms/components.h>
#include <Helium/smiles.h>


#include "test.h"

using namespace Helium;

void test_canonicalize(const std::string &smiles)
{
  std::cout << "Testing: " << smiles << std::endl;
  HeMol mol;
  try {
    parse_smiles(smiles, mol);
  } catch (Smiley::Exception &e) {
    std::cerr << e.what();
  }
  std::vector<unsigned long> symmetry = extended_connectivities(mol, AtomInvariant());
  std::cout << "symmetry: " << symmetry << std::endl;
  canonicalize(mol, symmetry, AtomInvariant(), BondInvariant());
}

bool shuffle_test_mol(HeMol &mol)
{
  bool pass = true;
  std::vector<Index> atoms;
  for (std::size_t i = 0; i < num_atoms(mol); ++i)
    atoms.push_back(i);


  std::pair<std::vector<Index>, std::vector<unsigned long> > ref_canon = canonicalize(mol,
        extended_connectivities(mol, AtomInvariant()), AtomInvariant(), BondInvariant());
  const std::vector<unsigned long> &ref_code = ref_canon.second;
  std::string ref_smiles = write_smiles(mol, ref_canon.first, WriteSmiles::All);

  // make sure the canonical SMILES is valid and can be read
  try {
  } catch (const Smiley::Exception &e) {
    std::cout << "invalid canonical SMILES:" << std::endl;
    std::cout << e.what() << std::endl;
    ASSERT(0);
    return false;
  }

  for (int i = 0; i < 10; ++i) {
    std::random_shuffle(atoms.begin(), atoms.end());
    //std::cout << "P: " << atoms << std::endl;
    //std::cout << mol << std::endl;
    mol.renumberAtoms(atoms);
    //std::cout << mol << std::endl;

    std::pair<std::vector<Index>, std::vector<unsigned long> > canon = canonicalize(mol,
        extended_connectivities(mol, AtomInvariant()), AtomInvariant(), BondInvariant());
    const std::vector<unsigned long> &code = canon.second;
    std::string smiles = write_smiles(mol, canon.first, WriteSmiles::All);

    std::cout << smiles << std::endl;

    COMPARE(ref_code, code);
    if (ref_code != code)
      pass = false;

    COMPARE(ref_smiles, smiles);
  }

  return pass;
}

void shuffle_test_smiles(const std::string &smiles)
{
  std::cout << "Testing " << smiles << "..." << std::endl;
  HeMol mol;
  parse_smiles(smiles, mol);
  shuffle_test_mol(mol);
}

void shuffle_test(const std::string &filename)
{
  std::cout << "Shuffle test: " << filename << std::endl;
  MoleculeFile file(filename);

  bool failed = false;
  unsigned int idx = -1, numAtoms = -1;

  HeMol mol;
  for (unsigned int i = 0; i < file.numMolecules(); ++i) {
    file.readMolecule(mol);
    //if (unique_elements(connected_bond_components(mol)) > 1)
    //  continue;
    //if ((i % 1000) == 0)
    //  std::cout << "Testing molecule " << i << "..." << std::endl;

    std::cout << "  testing: " << write_smiles(mol) << std::endl;

    if (!shuffle_test_mol(mol) && num_atoms(mol) < numAtoms) {
      numAtoms = num_atoms(mol);
      idx = i;
      failed = true;
    }
  }

  if (failed)
    std::cout << "index of smallest molecule that failed: " << idx << std::endl;
}

int main(int argc, char **argv)
{
  bool validate = false;
  if (argc == 2)
    validate = (std::string("-validate") == argv[1]);

  shuffle_test_smiles("Clc1ccc2c(CCN2C(=O)C)c1");

  test_canonicalize("CCC(C)C");
  test_canonicalize("CCC(C(C)C)C");
  test_canonicalize("c1ccccc1");


  shuffle_test_smiles("[Cl-].OC(=O)C(CS)[NH3+]");
  shuffle_test_smiles("Cl.NCc1ncc(Br)c(C)c1");
  shuffle_test_smiles("C=CCc1c(O)nc(C)nc1O");
  shuffle_test_smiles("Clc1ccc(cc1)Cc1c(C)nc(N)[nH]c1=O");
  shuffle_test_smiles("COc1cc(C)nc(Cl)n1");
  shuffle_test_smiles("C1CN1");
  shuffle_test_smiles("CCOC(C)OCC");

  shuffle_test(datadir() + "1K.hel");

  if (validate) {
    shuffle_test(datadir() + "canonmulti.hel");
    //shuffle_test(datadir() + "1M.hel");
  }
}
