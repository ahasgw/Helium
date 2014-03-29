/*
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
#ifndef HELIUM_SMILES_H
#define HELIUM_SMILES_H

#include <Helium/molecule.h>
#include <Helium/algorithms/dfs.h>
#include <Helium/element.h>

#include <smiley.h>

#include <sstream>


namespace Helium {

  namespace impl {

    template<typename EditableMoleculeType>
    struct SmileyCallback : public Smiley::CallbackBase
    {
      SmileyCallback(EditableMoleculeType &mol_) : mol(mol_)
      {
      }

      void clear()
      {
        clear_molecule(mol);
      }

      void addAtom(int element, bool aromatic, int isotope, int hCount, int charge, int atomClass)
      {
        typename molecule_traits<EditableMoleculeType>::atom_type atom = add_atom(mol);
        set_element(mol, atom, element);
        set_aromatic(mol, atom, aromatic);
        if (isotope != -1)
          set_mass(mol, atom, isotope);
        else
          set_mass(mol, atom, Element::averageMass(element));
        if (hCount != -1)
          set_hydrogens(mol, atom, hCount);
        else
          set_hydrogens(mol, atom, 99);
        set_charge(mol, atom, charge);
      }

      void addBond(int source, int target, int order, bool isUp, bool isDown)
      {
        typename molecule_traits<EditableMoleculeType>::bond_type bond = add_bond(mol, get_atom(mol, source), get_atom(mol, target));
        if (order == 5)
          set_aromatic(mol, bond, true);
        set_order(mol, bond, order);
      }

      EditableMoleculeType &mol;
    };

  }

  /**
   * @brief Parse a SMILES string.
   *
   * @tparam EditableMoleculeType The type of the molecule, this must be a model
   *         of the EdiatbleMolecule concept.
   *
   * @param smiles The SMILES string.
   * @param mol The molecule.
   */
  template<typename EditableMoleculeType>
  void parse_smiles(const std::string &smiles, EditableMoleculeType &mol)
  {
    impl::SmileyCallback<EditableMoleculeType> callback(mol);
    Smiley::Parser<impl::SmileyCallback<EditableMoleculeType> > parser(callback);

    try {
      parser.parse(smiles);
    } catch (Smiley::Exception &e) {
      std::ostringstream errorStream;
      if (e.type() == Smiley::Exception::SyntaxError)
        errorStream << "Syntax";
      else
        errorStream << "Semantics";
      errorStream << "Error: " << e.what() << "." << std::endl;
      errorStream << smiles << std::endl;
      for (std::size_t i = 0; i < e.pos(); ++i)
        errorStream << " ";
      for (std::size_t i = 0; i < e.length(); ++i)
        errorStream << "^";
      errorStream << std::endl;

      // Throw new exception with added details ...
      throw Smiley::Exception(e.type(), e.errorCode(),
          errorStream.str(), e.pos(), e.length());

    }

    // add hydrogens
    FOREACH_ATOM (atom, mol, EditableMoleculeType) {
      if (num_hydrogens(mol, *atom) != 99)
        continue;
      if (!Element::addHydrogens(get_element(mol, *atom)))
        continue;

      int explicitH = 0;
      FOREACH_NBR (nbr, *atom, mol, EditableMoleculeType)
        if (get_element(mol, *nbr) == 1)
          ++explicitH;

      int valence = get_valence(mol, *atom);
      int expValence = Element::valence(get_element(mol, *atom), get_charge(mol, *atom), valence);
      //std::cout << "val: " << valence << "  deg: " << valence << " explH: " << explicitH << std::endl;
      if (expValence > valence - explicitH)
        set_hydrogens(mol, *atom, expValence - valence);
      else
        set_hydrogens(mol, *atom, 0);
    }
  }

  namespace WriteSmiles {

    /**
     * @brief SMILES features to write.
     */
    enum Flags {
      None = 0,
      Mass = 1,
      Charge = 2,
      Hydrogens = 4,
      Order = 8,
      All = Mass | Charge | Hydrogens | Order
    };

  }

  namespace impl {

    template<typename MoleculeType>
    struct WriteSmilesRingNumberVisitor : public DFSVisitor<MoleculeType>
    {
      typedef typename molecule_traits<MoleculeType>::atom_type atom_type;
      typedef typename molecule_traits<MoleculeType>::bond_type bond_type;

      void initialize(const MoleculeType &mol)
      {
        ringNumber = 0;
        ringNumbers.clear();
      }

      void back_bond(const MoleculeType &mol, bond_type bond)
      {
        ++ringNumber;

        if (ringNumbers.find(get_source(mol, bond)) == ringNumbers.end())
          ringNumbers[get_source(mol, bond)] = std::vector<int>();
        ringNumbers[get_source(mol, bond)].push_back(ringNumber);

        if (ringNumbers.find(get_target(mol, bond)) == ringNumbers.end())
          ringNumbers[get_target(mol, bond)] = std::vector<int>();
        ringNumbers[get_target(mol, bond)].push_back(ringNumber);
      }

      int ringNumber;
      std::map<atom_type, std::vector<int> > ringNumbers;
    };

    template<typename MoleculeType>
    struct WriteSmilesVisitor : public DFSVisitor<MoleculeType>
    {
      typedef typename molecule_traits<MoleculeType>::atom_type atom_type;
      typedef typename molecule_traits<MoleculeType>::bond_type bond_type;

      WriteSmilesVisitor(const std::map<atom_type, std::vector<int> > &ringNumbers_, int flags_)
        : ringNumbers(ringNumbers_), explicitBond(0), flags(flags_)
      {
      }

      void initialize(const MoleculeType &mol)
      {
        degrees.resize(num_atoms(mol));
        explicitBond = 0;
      }

      bool isOrganicSubset(int element) const
      {
        switch (element) {
          case 5: // B
          case 6: // C
          case 7: // N
          case 8: // O
          case 9: // F
          case 15: // P
          case 16: // S
          case 17: // Cl
          case 35: // Br
          case 53: // I
            return true;
          default:
            return false;
        }
      }

      void atom(const MoleculeType &mol, atom_type prev, atom_type atom)
      {
        if (prev != molecule_traits<MoleculeType>::null_atom()) {
          typename std::map<atom_type, std::vector<int> >::const_iterator rings = ringNumbers.find(prev);
          int numRings = rings == ringNumbers.end() ? 0 : rings->second.size();
          degrees[get_index(mol, prev)]++;

          // the -1 comes from the previous atom of prev
          // (this is compensated below if prev is a first atom in it's component)
          if (degrees[get_index(mol, prev)] < get_degree(mol, prev) - 1 - numRings) {
            smiles << "(";
            branches.push_back(get_index(mol, atom));
          }
        } else {
          degrees[get_index(mol, atom)] = -1; // compensate for -1 above

          if (smiles.str().size())
            smiles << ".";
        }


        if (explicitBond)
          smiles << explicitBond;
        explicitBond = 0;

        std::string element = Element::symbol(get_element(mol, atom));
        if (is_aromatic(mol, atom))
          std::transform(element.begin(), element.end(), element.begin(), ::tolower);

        // ignore mass 0
        bool needBrackets = !isOrganicSubset(get_element(mol, atom));
        if (get_charge(mol, atom) && (flags & WriteSmiles::Charge))
          needBrackets = true;
        if (get_mass(mol, atom) && get_mass(mol, atom) != Element::averageMass(get_element(mol, atom)) && (flags & WriteSmiles::Mass))
          needBrackets = true;

        if (needBrackets)
          smiles << "[";

        if (get_mass(mol, atom) && get_mass(mol, atom) != Element::averageMass(get_element(mol, atom)) && (flags & WriteSmiles::Mass))
          smiles << get_mass(mol, atom);

        smiles << element;

        int numH = num_hydrogens(mol, atom);
        if (needBrackets && (flags & WriteSmiles::Hydrogens) && numH)
          smiles << "H" << numH;

        if ((flags & WriteSmiles::Charge) && get_charge(mol, atom)) {
          int charge = get_charge(mol, atom);
          if (charge == 1)
            smiles << "+";
          else if (charge == -1)
            smiles << "-";
          else if (charge > 0)
            smiles << "+" << charge;
          else
            smiles << "-" << charge;
        }


        if (needBrackets)
          smiles << "]";


        typename std::map<atom_type, std::vector<int> >::const_iterator rings = ringNumbers.find(atom);
        if (rings != ringNumbers.end())
          for (std::size_t i = 0; i < rings->second.size(); ++i) {
            if (rings->second[i] > 9)
              smiles << "%" << rings->second[i];
            else
              smiles << rings->second[i];
          }
      }

      void bond(const MoleculeType &mol, atom_type prev, bond_type bond)
      {
        if (!(flags & WriteSmiles::Order))
          return;

        explicitBond = 0;

        switch (get_order(mol, bond)) {
          case 1:
            if (!is_aromatic(mol, bond) && is_aromatic(mol, get_source(mol, bond)) && is_aromatic(mol, get_target(mol, bond)))
              explicitBond = '-';
            break;
          case 2:
            if (!is_aromatic(mol, bond))
              explicitBond = '=';
            break;
          case 3:
            explicitBond = '#';
            break;
          case 4:
            explicitBond = '$';
            break;
          default:
            break;
        }
      }

      void backtrack(const MoleculeType &mol, atom_type atom)
      {
        if (branches.size() && branches.back() == get_index(mol, atom)) {
          smiles << ")";
          branches.pop_back();
        }
      }

      void back_bond(const MoleculeType &mol, bond_type bond)
      {
      }

      const std::map<atom_type, std::vector<int> > &ringNumbers;
      std::vector<int> degrees;
      std::vector<Index> branches;
      std::stringstream smiles;
      char explicitBond;
      int flags;
    };

  }

  /**
   * @brief Write a SMILES string for the molecule.
   *
   * @param mol The molecule.
   * @param flags The SMILES features to write (see WriteSmiles::Flags).
   *
   * @return The SMILES string.
   */
  template<typename MoleculeType>
  std::string write_smiles(const MoleculeType &mol, int flags = WriteSmiles::All)
  {
    impl::WriteSmilesRingNumberVisitor<MoleculeType> ringNumbers;
    depth_first_search(mol, ringNumbers);

    impl::WriteSmilesVisitor<MoleculeType> smilesWriter(ringNumbers.ringNumbers, flags);
    depth_first_search(mol, smilesWriter);

    return smilesWriter.smiles.str();
  }

  /**
   * @brief Write a SMILES string for the molecule using a specified order.
   *
   * This version of write_smiles() can be used to write canonical SMILES if a
   * canonical atom order is used.
   *
   * @param mol The molecule.
   * @param order The atom order.
   * @param flags The SMILES features to write (see WriteSmiles::Flags).
   *
   * @return The SMILES string.
   */
  template<typename MoleculeType>
  std::string write_smiles(const MoleculeType &mol, const std::vector<Index> &order, int flags = WriteSmiles::All)
  {
    impl::WriteSmilesRingNumberVisitor<MoleculeType> ringNumbers;
    depth_first_search(mol, order, ringNumbers);

    impl::WriteSmilesVisitor<MoleculeType> smilesWriter(ringNumbers.ringNumbers, flags);
    depth_first_search(mol, order, smilesWriter);

    return smilesWriter.smiles.str();
  }

}

#endif
