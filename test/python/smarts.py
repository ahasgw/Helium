import helium
import unittest

SMILES = helium.Smiles()

class TestSmarts(unittest.TestCase):

    def test_valid_smarts(self):
        smarts = helium.Smarts()
        self.assertTrue(smarts.init('C'))
        self.assertFalse(smarts.error().__nonzero__())
        self.assertEqual(0, len(str(smarts.error())))

    def test_invalid_smarts(self):
        smarts = helium.Smarts()
        self.assertFalse(smarts.init('fdsgsgd'))
        self.assertTrue(smarts.error().__nonzero__())
        self.assertNotEqual(0, len(str(smarts.error())))

    def test_search_no_mapping_hit(self):
        smarts = helium.Smarts()
        smarts.init('C')

        mol = helium.Molecule()
        SMILES.read('CCC', mol)

        mapping = helium.NoMapping()
        rings = helium.RingSet(mol)

        self.assertTrue(smarts.search(mol, mapping, rings))
        self.assertTrue(mapping.match)

    def test_search_no_mapping_miss(self):
        smarts = helium.Smarts()
        smarts.init('N')

        mol = helium.Molecule()
        SMILES.read('CCC', mol)

        mapping = helium.NoMapping()
        rings = helium.RingSet(mol)

        self.assertFalse(smarts.search(mol, mapping, rings))
        self.assertFalse(mapping.match)

    def test_search_count_mapping(self):
        smarts = helium.Smarts()
        smarts.init('C')

        mol = helium.Molecule()
        SMILES.read('CCC', mol)

        mapping = helium.CountMapping()
        rings = helium.RingSet(mol)

        self.assertTrue(smarts.search(mol, mapping, rings))
        self.assertEqual(3, mapping.count)

    def test_search_single_mapping(self):
        smarts = helium.Smarts()
        smarts.init('c1ccccc1')

        mol = helium.Molecule()
        SMILES.read('c1ccccc1-c2ccccc2', mol)

        mapping = helium.SingleMapping()
        rings = helium.RingSet(mol)


        self.assertTrue(smarts.search(mol, mapping, rings))
        self.assertEqual(6, len(mapping.map))

    def test_search_mapping_list(self):
        smarts = helium.Smarts()
        smarts.init('c1ccccc1')

        mol = helium.Molecule()
        SMILES.read('c1ccccc1-c2ccccc2', mol)

        mapping = helium.MappingList()
        rings = helium.RingSet(mol)

        self.assertTrue(smarts.search(mol, mapping, rings))
        self.assertEqual(2, len(mapping.maps))
        self.assertEqual(6, len(mapping.maps[0]))
        self.assertEqual(6, len(mapping.maps[1]))

    def test_search_unqiue(self):
        smarts = helium.Smarts()
        smarts.init('C.C')

        mol = helium.Molecule()
        SMILES.read('C.C', mol)

        mapping = helium.MappingList()
        rings = helium.RingSet(mol)

        self.assertTrue(smarts.search(mol, mapping, rings))
        self.assertEqual(1, len(mapping.maps))
        self.assertEqual(2, len(mapping.maps[0]))

        self.assertTrue(smarts.search(mol, mapping, rings, False))
        self.assertEqual(3, len(mapping.maps))
        self.assertEqual(2, len(mapping.maps[0]))
        self.assertEqual(2, len(mapping.maps[1]))
        self.assertEqual(2, len(mapping.maps[2]))

    def test_requires_cycles(self):
        smarts = helium.Smarts()

        smarts.init('C')
        self.assertFalse(smarts.requiresCycles())

        smarts.init('[CR]')
        self.assertTrue(smarts.requiresCycles())

    def test_requires_explicit_hydrogens(self):
        smarts = helium.Smarts()

        smarts.init('C')
        self.assertFalse(smarts.requiresExplicitHydrogens())

        smarts.init('[C]([H])')
        self.assertTrue(smarts.requiresExplicitHydrogens())

if __name__ == '__main__':
    unittest.main()
