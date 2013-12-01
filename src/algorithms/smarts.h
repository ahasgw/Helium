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
#ifndef HELIUM_SMARTS_H
#define HELIUM_SMARTS_H

#include <iostream>

#include <Helium/hemol.h>
#include <Helium/ring.h>
#include <Helium/algorithms/isomorphism.h>
#include <Helium/algorithms/components.h>

#include <Helium/smiles.h> // FIXME remove debug

#include <smiley.h>

namespace Helium {

  namespace impl {

    struct SmartsAtomExpr
    {
      SmartsAtomExpr(int type_)
        : type(type_)
      {
      }
      SmartsAtomExpr(int type_, int value_)
        : type(type_), value(value_)
      {
      }
      SmartsAtomExpr(int type_, SmartsAtomExpr *arg_)
        : type(type_), arg(arg_)
      {
      }
      SmartsAtomExpr(int type_, SmartsAtomExpr *left_, SmartsAtomExpr *right_)
        : type(type_), left(left_), right(right_)
      {
      }

      int type;
      union {
        int value;
        SmartsAtomExpr *arg;
        SmartsAtomExpr *left;
      };
      SmartsAtomExpr *right;
    };

    struct SmartsBondExpr
    {
      SmartsBondExpr(int type_)
        : type(type_)
      {
      }
      SmartsBondExpr(int type_, SmartsBondExpr *arg_)
        : type(type_), arg(arg_)
      {
      }
      SmartsBondExpr(int type_, SmartsBondExpr *left_, SmartsBondExpr *right_)
        : type(type_), left(left_), right(right_)
      {
      }


      int type;
      union {
        int value;
        SmartsBondExpr *arg;
        SmartsBondExpr *left;
      };
      SmartsBondExpr *right;
    };

    class SmartsTrees
    {
      public:
        SmartsTrees();

        SmartsTrees(const SmartsTrees &other);

        ~SmartsTrees();

        SmartsTrees& operator=(const SmartsTrees &other);

        void addAtom(SmartsAtomExpr *expr)
        {
          m_atoms.push_back(expr);
        }

        SmartsAtomExpr* atom(std::size_t index) const
        {
          return m_atoms[index];
        }

        const std::vector<SmartsAtomExpr*>& atoms() const
        {
          return m_atoms;
        }

        void addBond(SmartsBondExpr *expr)
        {
          m_bonds.push_back(expr);
        }

        SmartsBondExpr* bond(std::size_t index) const
        {
          return m_bonds[index];
        }

        const std::vector<SmartsBondExpr*>& bonds() const
        {
          return m_bonds;
        }

        SmartsAtomExpr* copy(SmartsAtomExpr *expr);

        SmartsBondExpr* copy(SmartsBondExpr *expr);

      private:
        template<typename ExprType>
        void cleanup(ExprType *expr)
        {
          switch (expr->type) {
            case Smiley::OP_AndHi:
            case Smiley::OP_AndLo:
            case Smiley::OP_And:
            case Smiley::OP_Or:
              cleanup(expr->left);
              cleanup(expr->right);
              break;
            case Smiley::OP_Not:
              cleanup(expr->arg);
              break;
            default:
              break;
          }
          delete expr;
        }

        std::vector<SmartsAtomExpr*> m_atoms;
        std::vector<SmartsBondExpr*> m_bonds;
    };

    template<typename QueryType, typename MoleculeType>
    class SmartsBondMatcher;

    template<typename QueryType, typename MoleculeType>
    class SmartsAtomMatcher
    {
      public:
        typedef typename molecule_traits<MoleculeType>::atom_type atom_type;
        typedef typename molecule_traits<QueryType>::atom_type query_atom_type;

        SmartsAtomMatcher(const std::vector<SmartsAtomExpr*> &atoms, const RingSet<MoleculeType> &rings,
            const std::vector<HeMol> &recursiveMols, const std::vector<SmartsTrees> &recursiveTrees)
          : m_atoms(atoms), m_rings(rings), m_recursiveMols(recursiveMols),
            m_recursiveTrees(recursiveTrees)
        {
        }

        bool operator()(const QueryType &query, query_atom_type queryAtom,
            const MoleculeType &mol, atom_type atom) const
        {
          SmartsAtomExpr *expr = m_atoms[get_index(query, queryAtom)];
          return match(mol, atom, expr);
        }

      private:
        bool match(const MoleculeType &mol, atom_type atom, SmartsAtomExpr *expr) const
        {
          switch (expr->type) {
            case Smiley::AE_True:
              return true;
            case Smiley::AE_False:
              return false;
            case Smiley::AE_Aromatic:
              return is_aromatic(mol, atom);
            case Smiley::AE_Aliphatic:
              return !is_aromatic(mol, atom);
            case Smiley::AE_Cyclic:
              return m_rings.isAtomInRing(atom);
            case Smiley::AE_Acyclic:
              return !m_rings.isAtomInRing(atom);
            case Smiley::AE_Isotope:
              return get_mass(mol, atom) == expr->value;
            case Smiley::AE_AtomicNumber:
              return get_element(mol, atom) == expr->value;
            case Smiley::AE_AromaticElement:
              return get_element(mol, atom) == expr->value && is_aromatic(mol, atom);
            case Smiley::AE_AliphaticElement:
              return get_element(mol, atom) == expr->value && !is_aromatic(mol, atom);
            case Smiley::AE_Degree:
              return get_degree(mol, atom) == expr->value;
            case Smiley::AE_Valence:
              return get_valence(mol, atom) == expr->value;
            case Smiley::AE_Connectivity:
              return get_connectivity(mol, atom) == expr->value;
            case Smiley::AE_TotalH:
              {
                int h = 0;
                FOREACH_NBR (nbr, atom, mol, MoleculeType)
                  if (get_element(mol, *nbr) == 1)
                    ++h;
                return (h + num_hydrogens(mol, atom)) == expr->value;
              }
            case Smiley::AE_ImplicitH:
              if (expr->value == -1) // default: at least 1
                return num_hydrogens(mol, atom) >= 1;
              else
                return num_hydrogens(mol, atom) == expr->value;
            case Smiley::AE_RingMembership:
              return m_rings.numRings(atom) == expr->value;
            case Smiley::AE_RingSize:
              return m_rings.isAtomInRingSize(atom, expr->value);
            case Smiley::AE_RingConnectivity:
              if (expr->value == -1) // default: at least 1
                return m_rings.numRingBonds(atom) >= 1;
              else
                return m_rings.numRingBonds(atom) == expr->value;
            case Smiley::AE_Charge:
              return get_charge(mol, atom) == expr->value;
            case Smiley::AE_Chirality:
              return true;
            case Smiley::AE_AtomClass:
              return true;
            case Smiley::AE_Recursive:
              {
                assert(expr->value < m_recursiveMols.size());
                assert(expr->value < m_recursiveTrees.size());
                impl::SmartsAtomMatcher<HeMol, MoleculeType> atomMatcher(m_recursiveTrees[expr->value].atoms(),
                    m_rings, m_recursiveMols, m_recursiveTrees);
                impl::SmartsBondMatcher<HeMol, MoleculeType> bondMatcher(m_recursiveTrees[expr->value].bonds(), m_rings);
                NoMapping mapping;
                return isomorphism_search(mol, atom, m_recursiveMols[expr->value], mapping, atomMatcher, bondMatcher);
              }
            case Smiley::OP_Not:
              return !match(mol, atom, expr->arg);
            case Smiley::OP_AndHi:
            case Smiley::OP_AndLo:
            case Smiley::OP_And:
              return match(mol, atom, expr->left) && match(mol, atom, expr->right);
            case Smiley::OP_Or:
              return match(mol, atom, expr->left) || match(mol, atom, expr->right);
            default:
              return true;
          }
        }

        const std::vector<SmartsAtomExpr*> m_atoms;
        const RingSet<MoleculeType> &m_rings;
        const std::vector<HeMol> &m_recursiveMols;
        const std::vector<SmartsTrees> &m_recursiveTrees;
    };

    template<typename QueryType, typename MoleculeType>
    class SmartsBondMatcher
    {
      public:
        typedef typename molecule_traits<MoleculeType>::bond_type bond_type;
        typedef typename molecule_traits<QueryType>::bond_type query_bond_type;

        SmartsBondMatcher(const std::vector<SmartsBondExpr*> &bonds, const RingSet<MoleculeType> &rings)
          : m_bonds(bonds), m_rings(rings)
        {
        }

        bool operator()(const QueryType &query, query_bond_type queryBond,
            const MoleculeType &mol, bond_type bond) const
        {
          SmartsBondExpr *expr = m_bonds[get_index(query, queryBond)];
          return match(mol, bond, expr);
        }

      private:
        bool match(const MoleculeType &mol, bond_type bond, SmartsBondExpr *expr) const
        {
          switch (expr->type) {
            case Smiley::BE_True:
              return true;
            case Smiley::BE_False:
              return false;
            case Smiley::BE_Single:
              return get_order(mol, bond) == 1;
            case Smiley::BE_Double:
              return get_order(mol, bond) == 2;
            case Smiley::BE_Triple:
              return get_order(mol, bond) == 3;
            case Smiley::BE_Quadriple:
              return get_order(mol, bond) == 4;
            case Smiley::BE_Aromatic:
              return is_aromatic(mol, bond);
            case Smiley::BE_Up:
            case Smiley::BE_Down:
              return true;
            case Smiley::BE_Ring:
              return m_rings.isBondInRing(bond);
            case Smiley::OP_Not:
              return !match(mol, bond, expr->arg);
            case Smiley::OP_AndHi:
            case Smiley::OP_AndLo:
            case Smiley::OP_And:
              return match(mol, bond, expr->left) && match(mol, bond, expr->right);
            case Smiley::OP_Or:
              return match(mol, bond, expr->left) || match(mol, bond, expr->right);
            default:
              return true;
          }
        }

        const std::vector<SmartsBondExpr*> m_bonds;
        const RingSet<MoleculeType> &m_rings;
    };

  }

  class Smarts
  {
    public:
      bool init(const std::string &smarts);

      const HeMol& query(std::size_t index = 0) const
      {
        assert(m_query.size() == 1);
        return m_query[index];
      }

      const impl::SmartsTrees& trees(std::size_t index = 0) const
      {
        assert(m_trees.size() == 1);
        return m_trees[index];
      }

      const std::vector<HeMol>& recursiveMols() const
      {
        return m_recursiveMols;
      }

      const std::vector<impl::SmartsTrees>& recursiveTrees() const
      {
        return m_recursiveTrees;
      }

      const Smiley::Exception& error() const
      {
        return m_error;
      }

      int atomClass(Index index) const;

      template<typename MoleculeType, typename MappingType>
      bool search(MoleculeType &mol, MappingType &mapping, const RingSet<MoleculeType> &rings);

      template<typename MoleculeType>
      bool search(MoleculeType &mol, const RingSet<MoleculeType> &rings)
      {
        NoMapping mapping;
        return search(mol, mapping, rings);
      }

    private:
      std::vector<HeMol> m_query;
      std::vector<impl::SmartsTrees> m_trees;
      std::vector<HeMol> m_recursiveMols;
      std::vector<impl::SmartsTrees> m_recursiveTrees;
      std::vector<std::vector<Index> > m_atomMaps; // m_query atom index to original smarts index
      //std::vector<std::vector<Index> > m_bondMaps; // m_query bond index to original smarts index
      Smiley::Exception m_error;
  };

  namespace impl {

    inline bool mappings_overlap(const IsomorphismMapping &map1, const IsomorphismMapping &map2)
    {
      for (std::size_t i = 0; i < map1.size(); ++i)
        if (std::find(map2.begin(), map2.end(), map1[i]) != map2.end())
          return true;
      return false;
    }

    template<typename MappingType>
    void enumerate_mappings(std::size_t fragment, const std::vector<MappingList> &mappings,
        const IsomorphismMapping &current, const std::vector<std::vector<Index> > atomMaps,
        MappingType &output, bool &match)
    {
      for (std::size_t i = 0; i < mappings[fragment].maps.size(); ++i) {
        // check for overlap
        if (mappings_overlap(current, mappings[fragment].maps[i]))
          continue;
        // add mapping of considered fragment to current by translating indices
        IsomorphismMapping map = current;
        const IsomorphismMapping &next = mappings[fragment].maps[i];
        for (std::size_t j = 0; j < next.size(); ++j)
          map[atomMaps[fragment][j]] = next[j];

        if (fragment + 1 < mappings.size()) {
          // recursive call to consider next fragment
          enumerate_mappings(fragment + 1, mappings, map, atomMaps, output, match);
        } else {
          // last fragment done, add mapping to output
          assert(std::find(map.begin(), map.end(), -1) == map.end());
          impl::add_mapping(output, map);
          match = true;
        }
      }
    }

  }

  template<typename MoleculeType, typename MappingType>
  bool Smarts::search(MoleculeType &mol, MappingType &mapping, const RingSet<MoleculeType> &rings)
  {
    if (m_query.empty())
      return false;

    if (m_query.size() == 1) {
      // simple case: single SMARTS fragment
      impl::SmartsAtomMatcher<HeMol, MoleculeType> atomMatcher(m_trees[0].atoms(),
          rings, m_recursiveMols, m_recursiveTrees);
      impl::SmartsBondMatcher<HeMol, MoleculeType> bondMatcher(m_trees[0].bonds(), rings);
      return isomorphism_search(mol, m_query[0], mapping, atomMatcher, bondMatcher);
    } else {
      // match each fragment seperatly and store results in mappings
      int numQueryAtoms = 0;
      std::vector<MappingList> mappings(m_query.size());
      for (std::size_t i = 0; i < m_query.size(); ++i) {
        numQueryAtoms += num_atoms(m_query[i]);
        impl::SmartsAtomMatcher<HeMol, MoleculeType> atomMatcher(m_trees[i].atoms(),
            rings, m_recursiveMols, m_recursiveTrees);
        impl::SmartsBondMatcher<HeMol, MoleculeType> bondMatcher(m_trees[i].bonds(), rings);
        if (!isomorphism_search(mol, m_query[i], mappings[i], atomMatcher, bondMatcher))
          return false;
      }

      bool match = false;
      IsomorphismMapping map(numQueryAtoms, -1);
      impl::enumerate_mappings(0, mappings, map, m_atomMaps, mapping, match);

      return match;
    }
  }

}

#endif
