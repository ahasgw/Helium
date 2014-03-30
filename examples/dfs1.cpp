// examples/dfs1.cpp
#include <Helium/algorithms/dfs.h>
#include <Helium/hemol.h>
#include <Helium/smiles.h>

using namespace Helium;

int main()
{
  HeMol mol;
  Smiles SMILES;
  if (!SMILES.read("C1CCC(CC)CC1", mol)) {
    std::cerr << SMILES.error().what();
    return -1;
  }

  DFSDebugVisitor<HeMol> visitor;
  depth_first_search(mol, visitor);
}
