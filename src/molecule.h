/**
 * Copyright (c) 2013, Tim Vandermeersch
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef HELIUM_MOLECULE_H
#define HELIUM_MOLECULE_H

#include <utility>

namespace Helium {

  /**
   * Type used for atom and bond indices.
   */
  typedef unsigned int Index;
  /**
   * Type used for various sizes (number of atoms/bonds, number of neighbors,
   * ...).
   */
  typedef unsigned int Size;

  //@cond dev

  template<typename MoleculeType>
  struct molecule_traits
  {
    typedef typename MoleculeType::atom_type atom_type;
    typedef typename MoleculeType::bond_type bond_type;

    typedef typename MoleculeType::mol_atom_iter mol_atom_iter;
    typedef typename MoleculeType::mol_bond_iter mol_bond_iter;
    typedef typename MoleculeType::atom_atom_iter atom_atom_iter;
    typedef typename MoleculeType::atom_bond_iter atom_bond_iter;

    static Index null_index()
    {
      return MoleculeType::null_index();
    }

    static atom_type null_atom()
    {
      return MoleculeType::null_atom();
    }

    static bond_type null_bond()
    {
      return MoleculeType::null_bond();
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  //
  // Molecule
  //
  //////////////////////////////////////////////////////////////////////////////

  /////////////////////////////
  //
  // Atoms
  //
  /////////////////////////////

  template<typename MoleculeType>
  Size num_atoms(const MoleculeType *mol);

  template<typename MoleculeType>
  std::pair<typename molecule_traits<MoleculeType>::mol_atom_iter, typename molecule_traits<MoleculeType>::mol_atom_iter>
  get_atoms(MoleculeType *mol);

  template<typename MoleculeType>
  typename molecule_traits<MoleculeType>::atom_type get_atom(const MoleculeType *mol, Index index);

  /////////////////////////////
  //
  // Bonds
  //
  /////////////////////////////

  template<typename MoleculeType>
  Size num_bonds(const MoleculeType *mol);

  template<typename MoleculeType>
  std::pair<typename molecule_traits<MoleculeType>::mol_bond_iter, typename molecule_traits<MoleculeType>::mol_bond_iter>
  get_bonds(MoleculeType *mol);

  template<typename MoleculeType>
  typename molecule_traits<MoleculeType>::bond_type get_bond(const MoleculeType *mol, Index index);

  template<typename MoleculeType>
  typename molecule_traits<MoleculeType>::bond_type get_bond(const MoleculeType *mol, typename molecule_traits<MoleculeType>::atom_type source,
                                                                                      typename molecule_traits<MoleculeType>::atom_type target);

  //////////////////////////////////////////////////////////////////////////////
  //
  // Atom
  //
  //////////////////////////////////////////////////////////////////////////////

  template<typename MoleculeType>
  Index get_index(const MoleculeType *mol, typename molecule_traits<MoleculeType>::atom_type atom);

  template<typename MoleculeType>
  std::pair<typename molecule_traits<MoleculeType>::atom_bond_iter, typename molecule_traits<MoleculeType>::atom_bond_iter>
  get_bonds(const MoleculeType *mol, typename molecule_traits<MoleculeType>::atom_type atom);

  template<typename MoleculeType>
  std::pair<typename molecule_traits<MoleculeType>::atom_atom_iter, typename molecule_traits<MoleculeType>::atom_atom_iter>
  get_nbrs(const MoleculeType *mol, typename molecule_traits<MoleculeType>::atom_type atom);

  template<typename MoleculeType>
  bool is_aromatic(const MoleculeType *mol, typename molecule_traits<MoleculeType>::atom_type atom);

  template<typename MoleculeType>
  bool is_cyclic(const MoleculeType *mol, typename molecule_traits<MoleculeType>::atom_type atom);

  template<typename MoleculeType>
  int get_element(const MoleculeType *mol, typename molecule_traits<MoleculeType>::atom_type atom);

  template<typename MoleculeType>
  int get_mass(const MoleculeType *mol, typename molecule_traits<MoleculeType>::atom_type atom);

  template<typename MoleculeType>
  int get_degree(const MoleculeType *mol, typename molecule_traits<MoleculeType>::atom_type atom);

  template<typename MoleculeType>
  int num_hydrogens(const MoleculeType *mol, typename molecule_traits<MoleculeType>::atom_type atom);

  template<typename MoleculeType>
  int get_charge(const MoleculeType *mol, typename molecule_traits<MoleculeType>::atom_type atom);

  //////////////////////////////////////////////////////////////////////////////
  //
  // Bond
  //
  //////////////////////////////////////////////////////////////////////////////

  template<typename MoleculeType>
  Index get_index(const MoleculeType *mol, typename molecule_traits<MoleculeType>::bond_type bond);

  template<typename MoleculeType>
  typename molecule_traits<MoleculeType>::atom_type get_source(const MoleculeType *mol, typename molecule_traits<MoleculeType>::bond_type bond);

  template<typename MoleculeType>
  typename molecule_traits<MoleculeType>::atom_type get_target(const MoleculeType *mol, typename molecule_traits<MoleculeType>::bond_type bond);

  template<typename MoleculeType>
  typename molecule_traits<MoleculeType>::atom_type get_other(const MoleculeType *mol, typename molecule_traits<MoleculeType>::bond_type bond,
                                                                                       typename molecule_traits<MoleculeType>::atom_type atom);

  template<typename MoleculeType>
  bool is_aromatic(const MoleculeType *mol, typename molecule_traits<MoleculeType>::bond_type bond);

  template<typename MoleculeType>
  bool is_cyclic(const MoleculeType *mol, typename molecule_traits<MoleculeType>::bond_type bond);

  template<typename MoleculeType>
  bool get_order(const MoleculeType *mol, typename molecule_traits<MoleculeType>::bond_type bond);

  //@endcond

}

#endif
