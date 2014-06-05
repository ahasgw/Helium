/*
 * Copyright (C) 2009-2010,2014 by Tim Vandermeersch
 * Some portions Copyright (C) 2009 by Chris Morley
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
#ifndef HELIUM_DEPICT_H
#define HELIUM_DEPICT_H

#include <Helium/depict/painter.h>
#include <Helium/molecule.h>
#include <Helium/ring.h>
#include <Eigen/Core>

namespace Helium
{

  /**
   * @class Depict depict.h <Helium/depict.h>
   * @brief 2D depiction of molecules using Painter.
   */
  class Depict
  {
    public:
      enum AtomLabelType {
        AtomId = 1,
        AtomIndex,
        AtomSymmetryClass,
        AtomValence,
        AtomTetrahedralStereo
      };

      enum OptionType{
        bwAtoms              = 0x0001,
        internalColor        = 0x0002,
        noMargin             = 0x0004,
        drawTermC            = 0x0010,
        drawAllC             = 0x0020,
        noWedgeHashGen       = 0x0100,
        asymmetricDoubleBond = 0x0200
      };

      /**
       * Constructor.
       */
      Depict(Painter *painter);
      /**
       * Destructor.
       */
      ~Depict();
      /**
       * Draw @p mol using the painter previously stored in the constructor.
       *
       * @return True if successful.
       */

      template<typename MoleculeType>
      bool drawMolecule(const MoleculeType &mol, const RingSet<MoleculeType> &rings,
          const std::vector<std::pair<double, double> > &origCoords);

      /**
       * Draw atom labels of a specified @p type.
       *
       * @return True if successful.
       */
      //bool addAtomLabels(AtomLabelType type);

      void setBondLength(double length)
      {
        m_bondLength= length;
      }

      double bondLength() const
      {
        return m_bondLength;
      }

      void setPenWidth(double width)
      {
        m_penWidth = width;
        m_painter->setPenWidth(width);
      }

      double penWidth() const
      {
        return m_penWidth;
      }

      void setBondSpacing(double spacing)
      {
        m_bondSpacing = spacing;
      }

      double bondSpacing() const
      {
        return m_bondSpacing;
      }

      void setBondWidth(double width)
      {
        m_bondWidth = width;
      }

      double bondWidth() const
      {
        return m_bondWidth;
      }

      //void SetDrawingTerminalCarbon(bool enabled);
      //bool GetDrawingTerminalCarbon() const;

      //extendable with binary compatibility
      void setOption(unsigned opts)
      {
        m_options |= opts;
      }

      unsigned options() const
      {
        return m_options;
      }

      void clearOptions()
      {
        m_options = 0;
      }

      void setFontFamily(const std::string &family)
      {
        m_fontFamily = family;
        m_painter->setFontFamily(family);
      }

      const std::string& GetFontFamily() const
      {
        return m_fontFamily;
      }

      void setFontSize(int pointSize, bool subscript = false);
      int fontSize(bool subscript = false) const;

      void setAliasMode(bool b = true)
      {
        m_aliasMode = b;
      }

      //Color is not quite properly integrated into Depict, but is needed if
      //element-dependent coloring is to be used.
      void setBondColor(const std::string& scolor)
      {
        m_bondColor = scolor;
      }

    private:
      void drawSimpleBond(Eigen::Vector2d begin, Eigen::Vector2d end,
          bool beginLbl, bool endLbl, int beginValence, int endValence, int order,
          bool crossed_dbl_bond = false);
      void drawRingBond(Eigen::Vector2d begin, Eigen::Vector2d end,
          bool beginLbl, bool endLbl, int beginValence, int endValence, int order,
          const Eigen::Vector2d &center);
      void drawWedge(Eigen::Vector2d begin, Eigen::Vector2d end,
          bool beginLbl, bool endLbl);
      void drawHash(Eigen::Vector2d begin, Eigen::Vector2d end,
          bool beginLbl, bool endLbl);
      void drawWobblyBond(Eigen::Vector2d begin, Eigen::Vector2d end,
          bool beginLbl, bool endLbl);
      void drawAtomLabel(const std::string &label, int alignment, const Eigen::Vector2d &pos);

      template<typename MoleculeType, typename AtomType>
      bool hasLabel(const MoleculeType &mol, const AtomType &atom);
      //void setWedgeAndHash(OBMol* mol);

      Painter *m_painter;
      double m_bondLength;
      double m_penWidth;
      double m_bondSpacing;
      double m_bondWidth;
      //bool m_drawTerminalC;
      int m_fontSize, m_subscriptSize;
      bool m_aliasMode;
      std::string m_fontFamily;
      Color m_bondColor;
      unsigned m_options;
  };

  template<typename MoleculeType, typename AtomType>
  bool Depict::hasLabel(const MoleculeType &mol, const AtomType &atom)
  {
    if (!is_carbon(mol, atom))
      return true;
    if ((m_options & Depict::drawAllC) || ((m_options & Depict::drawTermC) && (get_valence(mol, atom) == 1)))
      return true;
    return false;
  }

  namespace impl {

    template<typename MoleculeType, typename AtomType>
    int labelAlignment(const MoleculeType &mol, const AtomType &atom,
        const std::vector<Eigen::Vector2d> &coords)
    {
      enum {
        Left,
        Right,
        Up,
        Down
      };

      // compute the sum of the bond vectors, this gives
      Eigen::Vector2d direction(Eigen::Vector2d::Zero());
      FOREACH_NBR_T (nbr, atom, mol, MoleculeType)
        direction += coords[get_index(mol, atom)] - coords[get_index(mol, *nbr)];

      const double bias = -0.1; //towards left-alignment, which is more natural
      int alignment = 0;
      if ((get_valence(mol, atom) == 2) && (abs(direction.y()) > abs(direction.x()))) {
        if (direction.y() <= 0.0)
          alignment = Up;
        else
          alignment = Down;
      } else {
        if (direction.x() < bias)
          alignment = Right;
        else
          alignment = Left;
      }

      return alignment;
    }

    Color elementColor(int element);

  }

  template<typename MoleculeType>
  bool Depict::drawMolecule(const MoleculeType &mol, const RingSet<MoleculeType> &rings,
      const std::vector<std::pair<double, double> > &origCoords)
  {
    typedef typename molecule_traits<MoleculeType>::atom_type atom_type;
    typedef typename molecule_traits<MoleculeType>::bond_type bond_type;

    if (!m_painter)
      return false;

    enum {
      Left,
      Right,
      Up,
      Down
    };

    double width = 0.0;
    double height = 0.0;

    // Determine which should be wedge and hash bonds...
    // Note: we need to do this before we invert the y-coordinate for depiction
    /* FIXME: stereo
    std::map<OBBond*, enum OBStereo::BondDirection> updown;
    std::map<OBBond*, OBStereo::Ref> from;
    TetStereoToWedgeHash(*d->mol, updown, from);
    */

    std::vector<Eigen::Vector2d> coords;
    if (num_atoms(mol) > 0) {
      for (std::size_t i = 0; i < origCoords.size(); ++i)
        coords.push_back(Eigen::Vector2d(origCoords[i].first, origCoords[i].second));

      // scale bond lengths and invert the y coordinate (both SVG and Cairo use top left as the origin)
      double bondLengthSum = 0.0;
      FOREACH_BOND_T (bond, mol, MoleculeType) {
        Index source = get_index(mol, get_source(mol, *bond));
        Index target = get_index(mol, get_target(mol, *bond));
        bondLengthSum += (coords[source] - coords[target]).norm();
      }
      const double averageBondLength = bondLengthSum / num_bonds(mol);
      const double f = num_bonds(mol) ? m_bondLength / averageBondLength : 1.0;

      for (std::size_t i = 0; i < coords.size(); ++i) {
        coords[i] *= f;
        coords[i].y() = coords[i].y();
      }

      // find min/max values
      double min_x = std::numeric_limits<double>::max();
      double max_x = std::numeric_limits<double>::min();
      double min_y = std::numeric_limits<double>::max();
      double max_y = std::numeric_limits<double>::min();
      for (std::size_t i = 0; i < coords.size(); ++i) {
        min_x = std::min(min_x, coords[i].x());
        max_x = std::max(max_x, coords[i].x());
        min_y = std::min(min_y, coords[i].y());
        max_y = std::max(max_y, coords[i].y());
      }

      double margin;
      if (m_options & noMargin)
        margin = 5.0;
      else
        margin = 40.0;

      // translate all atoms so the bottom-left atom is at margin,margin
      for (std::size_t i = 0; i < coords.size(); ++i) {
        coords[i].x() = coords[i].x() - min_x + margin;
        coords[i].y() = coords[i].y() - min_y + margin;
      }

      width  = max_x - min_x + 2 * margin;
      height = max_y - min_y + 2 * margin;

      //m_painter->setPenWidth(m_penWidth);
      //m_painter->setPenColor(m_pen));
      //m_painter->setFillColor(Color("black"));
    }

    m_painter->newCanvas(width, height);

    // Identify and remember the ring bonds according to the relevant cycles
    std::vector<bool> ringBonds(num_bonds(mol));
    for (std::size_t i = 0; i < rings.size(); ++i) {
      const Ring<MoleculeType> &ring = rings.ring(i);
      for (std::size_t j = 0; j < ring.size(); ++j)
        ringBonds[get_index(mol, ring.bonds()[j])] = true;
    }

    // draw bonds
    FOREACH_BOND_T (bond, mol, MoleculeType) {
      atom_type source = get_source(mol, *bond);
      atom_type target = get_target(mol, *bond);

      m_painter->setPenColor(m_bondColor);

      /* FIXME: stereo
      if(from.find(bond)!=from.end()) {
        //is a wedge or hash bond
        if(from[bond]==bond->GetEndAtom()->GetId())
          swap(begin, end);
        if(updown[bond]==OBStereo::UpBond)
          drawWedge(begin, end);
        else if(updown[bond]==OBStereo::DownBond)
          drawHash(begin, end);
        else {
          //This is a bond to a chiral center specified as unknown
          drawWobblyBond(begin, end);
        }
      }
      else */
      if (!ringBonds[get_index(mol, *bond)]) { // Ring bonds are handled below
        bool crossed_dbl_bond = false;
        /* FIXME: stereo
        OBStereoFacade sf(d->mol);
        if (sf.HasCisTransStereo(bond->GetId())) {
          OBCisTransStereo *ct = sf.GetCisTransStereo(bond->GetId());
          if (!ct->GetConfig().specified)
            crossed_dbl_bond = true;
        }
        */
        drawSimpleBond(coords[get_index(mol, source)], coords[get_index(mol, target)],
            hasLabel(mol, source), hasLabel(mol, target), get_valence(mol, source),
            get_valence(mol, target), get_order(mol, *bond), crossed_dbl_bond);
      }
    }

    // draw ring bonds
    m_painter->setPenColor(m_bondColor);
    std::vector<bool> drawnBonds(num_bonds(mol));
    for (std::size_t i = 0; i < rings.size(); ++i) {
      const Ring<MoleculeType> &ring = rings.ring(i);

      // compute center
      Eigen::Vector2d center(Eigen::Vector2d::Zero());
      for (std::size_t j = 0; j < ring.size(); ++j)
        center += coords[get_index(mol, ring.atom(j))];
      center /= ring.size();

      bool aromatic = true;
      for (std::size_t j = 0; j < ring.size(); ++j) {
        bond_type ringBond = ring.bond(j);
        atom_type source = get_source(mol, ringBond);
        atom_type target = get_target(mol, ringBond);

        if (drawnBonds[get_index(mol, ringBond)])
          continue;
        drawnBonds[get_index(mol, ringBond)] = true;

        if (get_order(mol, ringBond) != 5)
          aromatic = false;

        drawRingBond(coords[get_index(mol, source)], coords[get_index(mol, target)],
            hasLabel(mol, source), hasLabel(mol, target), get_valence(mol, source),
            get_valence(mol, target), get_order(mol, ringBond), center);
      }

      if (aromatic) {
        double minDist = std::numeric_limits<double>::max();
        for (std::size_t j = 0; j < ring.size(); ++j)
          minDist = std::min(minDist, (center - coords[get_index(mol, ring.atom(j))]).norm());

        m_painter->drawCircle(center.x(), center.y(), minDist - 3 * m_bondSpacing);
      }

    }

    // draw atom labels
    FOREACH_ATOM_T (atom, mol, MoleculeType) {
      double x = coords[get_index(mol, *atom)].x();
      double y = coords[get_index(mol, *atom)].y();

      int alignment = impl::labelAlignment(mol, *atom, coords);
      bool rightAligned = false;
      switch (alignment) {
        case Right:
          rightAligned = true;
        default:
          break;
      }

      if(m_options & bwAtoms)
        m_painter->setPenColor(m_bondColor);
      else
        m_painter->setPenColor(impl::elementColor(get_element(mol, *atom)));

      //charge and radical
      int charge = get_charge(mol, *atom);
      int spin = 0; //atom->GetSpinMultiplicity(); FIXME: radical
      if (charge || spin) {
        FontMetrics metrics = m_painter->fontMetrics("N");
        double yoffset = hasLabel(mol, *atom) ? -0.2 * metrics.height : -0.2 * metrics.height;
        /*switch (GetLabelAlignment(atom)) {
          case Up:
          case Left:
          case Right:
            yoffset = - 1.2 * metrics.height;
        }*/
        std::stringstream ss;
        if(charge) {
          if(abs(charge)!=1)
            ss << abs(charge);
          if(charge>0)
            ss << '+';
          else if (charge<-1) //use underscore for single negative charge and minus if multiple
            ss << '-';
          else
          {
            ss << '_';
            yoffset -= 0.5 * metrics.height;
          }
        }
        m_painter->drawText(x + 0.4*metrics.width, y+yoffset, ss.str());
        if(spin) {
          std::string radchars = (spin==2 ? "." : "..");
          //yoffset += 0.5 * metrics.height;
          m_painter->setFontSize(2 * metrics.fontSize);
          m_painter->drawText(x + (0.4 + ss.str().size())*metrics.width,
            y+yoffset, radchars);
        }
        m_painter->setFontSize(metrics.fontSize);//restore
      }

      if (is_carbon(mol, *atom)) {
        if(!(m_options & drawAllC))
        {
          if (get_valence(mol, *atom) > 1)
            continue;
          if ((get_valence(mol, *atom) == 1) && !(m_options & drawTermC))//!d->drawTerminalC)
            continue;
        }
      }

      std::stringstream ss;

      // atoms with element 0 are output as Rn
      if (get_element(mol, *atom) == 0) {
        ss << 'R';
        m_painter->setPenColor(Color("black"));
      } else {
        std::string atomSymbol;
        if (is_hydrogen(mol, *atom) && get_mass(mol, *atom) > 1)
          atomSymbol = get_mass(mol, *atom) == 2 ? "D" : "T";
        else
          atomSymbol = Element::symbol(get_element(mol, *atom));

        unsigned int hCount = get_hydrogens(mol, *atom);
        // rightAligned:
        //   false  CH3
        //   true   H3C
        if (hCount && rightAligned)
          ss << "H";
        if ((hCount > 1) && rightAligned)
          ss << hCount;
        ss << atomSymbol;
        if (hCount && !rightAligned)
          ss << "H";
        if ((hCount > 1) && !rightAligned)
          ss << hCount;
      }

      drawAtomLabel(ss.str(), alignment, Eigen::Vector2d(x, y));
    }

    return true;
  }

}

#endif
