#ifndef HELIUM_FILEIO_FINGERPRINTS_H
#define HELIUM_FILEIO_FINGERPRINTS_H

#include "../bitvec.h"
#include "file.h"

#include <json/json.h>

#include <stdexcept>

namespace Helium {

  /**
   * @page fingerprints_page Fingerprint Storage and Indexes
   *
   * @section fingerprints_files Fingerprint File Classes
   *
   *
   *
   *
   *
   * @section fingerprints_file_format Binary File Format
   * Like all Helium binary files, the fingerprint file formats include a JSON
   * header containing all information about the binary data contained in the
   * file. Below is an example of such a header.
   *
@code
{
  'filetype': 'fingerprints',
  'order': 'row-major',
  'num_bits': 1024,
  'num_fingerprints': 1000000,
  'fingerprint': {
    'name': 'My custom fingerprint',
    'type': 'Helium::trees_fingerprint',
    'k': 7,
    'prime': 1021
  }
}
@endcode
   *
   * For fingerprint files the 'filetype' attribute will always be 'fingerprints'.
   * The 'order', 'num_bits', 'num_fingerprints' and 'fingerprint' attributes are
   * mandatory.
   * In the 'fingerprint' attribute, only the 'name' and 'type' attributes are
   * mandatory. Additional attributes may always be added such as the 'k' and
   * 'prime' attributes in the example above. (note: These last two attributes
   * may be required by an application that needs to compute the fingerprint for
   * a query molecule)
   *
   * While the 'name' attribute is mainly meant for end user purposes, the 'type'
   * attribute should be a string that corresponds to the method used for
   * generating the fingerprints (e.g. 'MyChemLib::some_fingerprint'). The
   * remaining attributes may then be used to specify parameters for this
   * fingerprint type.
   *
   * @section fingerprints_rowcol Row-Major vs. Column-Major Order
   *
   * Fingerprints can be stored in row-major order (i.e. each fingerprint is
   * stored in consecutive memory) or in column-major order (i.e. each
   * fingerprint bit is stored consecutively in memory). To illustrate the
   * difference, consider storing these 8-bit fingerprints for four molecules:
   *
@verbatim
molecule 1: 00101100
molecule 2: 10001011
molecule 3: 00000110
molecule 4: 10101100
@endverbatim
   *
   * In the row-major order storage, the fingerprints are stored by concatenating
   * the above fingerprints. In the figure below, the addresses are bit addresses
   * in hexadecimal. Using this storage method, the bits of an individual
   * molecule's fingerprint are stored consecutively.
   *
@verbatim
0x00  00101100
0x08  10001011
0x10  00000110
0x18  10101100
@endverbatim
   *
   * In the column major order, the fingerprint are stored by concatenating the
   * columns. As a result, the individual fingerprint bits for all molecules are
   * stored consecutively. This can be seen in the figure below.
   *
@verbatim
0x00  0101
0x04  0000
0x08  1001
0x0C  0000
0x10  1101
0x14  1011
0x18  0110
0x1C  0100
@endverbatim
   *
   * The difference between these two methods of storing fingerprints can
   * dramatically influence the performance of an application. Due to various
   * hardware properties, sequential memory accesses are much faster than random
   * memory access patterns. Therefore it is vital to choose the correct storage
   * methods for each application.
   *
   * @section fingerprints_similarity Storing Fingerprints for Similarity Searches
   *
   * Similarity searches are performed by computing the similarity between the
   * query and queried molecules fingerprints. Although a variety of similarity
   * measures can be used the Tanimoto coefficient is very the most widely used.
   *
   * \f[
   *   T_{\mathrm{sim}} = \frac{| A \wedge B |}{| A \vee B |}
   * \f]
   *
   * Here \f$A\f$ and \f$B\f$ are the query and queried fingerprint bitstrings
   * and \f$|A|\f$ is the population count of fingerprint \f$A\f$.
   *
   * Since the whole fingerprint is needed to compute the similarity,
   * fingerprints for similarity searches are stored in row-major order.
   *
   * @section fingerprints_substructure Storing Fingerprints for Substructure Searches
   *
   * When performing a substructure search the fingerprints are used to filter out
   * molecules that will never match the query. All molecules that do not have
   * all bits set in their fingerprint that are set in the query's fingerprint
   * can never match the query. If the fingerprints (i.e. bitstrings) are
   * interpreted as sets where the value of the i-th bit would signify whether
   * the element i is in the set or not, this filtering can be seen as checking
   * if the query fingerprint is a subset of the molecule's fingerprint (which
   * would than be the superset).
   *
   * Let \f$M\f$ be set of all queried molecules\f$m\f$, \f$F\f$ be the set of all
   * possible fingerprints and \f$\mathrm{finger}: M \mapsto F\f$ be the function
   * assigning a fingerprint to a molecule. Then the set \f$I\f$ of molecules that
   * potentially contain the query is given by
   *
   * \f[
   *   I = \{ \forall m \in M \vert \mathrm{finger}(q) \subseteq \mathrm{finger}(m) \}
   * \f]
   *
   * where \f$q\f$ is the query.
   *
   * Constructing the set \f$I\f$ can easily be done by iterating over all
   * molecules (i.e. stored fingerprints) and adding them to \f$I\f$ if the
   * subset test passes. Although this would work, it is not very efficient.
   * A large portion of the data has to be processed that is not strictly
   * needed. A better way is to only check the bits that are set in the
   * query's fingerprint. While this is better in theory, care has to be taken
   * to implement this efficiently too. Checking only a single fingerprint bit
   * at a time using the row-major order storage would be very inefficient due
   * to random access memory patterns. To address this issue the column-major
   * order storage is used for substructure searching. It should also be noted
   * that constructing the final set \f$I\f$ can be done using the bitwise AND
   * operator on the bitstrings for the fingerprint bits that are set in the
   * query.
   *
   * A concrete example will illustrate how this works:
   *
@verbatim
query: 00001011 (bit 5, 7 & 8 are set)

0x00  0101
0x04  0000
0x08  1001
0x0C  0000
0x10  1101 <- bit 5
0x14  1011
0x18  0110 <- bit 7
0x1C  0100 <- bit 8

1101 & 0110 & 0100 = 0100 -> only molecule 2 can contain the query
@endverbatim
   *
   * In this example only 3 bitstrings have to be accessed. For this 8-bit
   * fingerprint this results in 62.5% less data that has to be processed.
   * Using a real fingerprint this may easily exceed 90% of the data (e.g.
   * a 1024-bit query fingerprint has 100 bits set).
   *
   * @section fingerprints_storage Fingerprint Storage Concepts
   *
   * In order for the indexing and search algorithms to work with various kind
   * of ways to actually store the fingerprints (e.g. read from disk on demand,
   * read from file and keep in main memory, memory mapped file, read from
   * database, ...), a concept for each major order is defined.
   *
   * @subsection fingerprints_storage_rowmajor Row-Major Order
   *
   * A class that is a model of the RowMajorFingerprintStorageConcept must support
   * the following operations:
   *
   * @code
   * std::string json = storage->header();
   * unsigned int n = storage->numBits();
   * unsigned int n = storage->numFingerprints();
   * Helium::Word *fingerprint = storage->fingerprint(index);
   * @endcode
   *
   * In the code above, storage is a pointer to an instance of a type that is a
   * model of the concept and index is an unsigned int. The index refers to the
   * molecule for which to get the fingerprint bitstring and should be in the
   * range [0, num_fingerprints). The returned pointer should point to a memory
   * location containing (for example) 1024 bits for a 1024-bit fingerprint.
   *
   * @subsection fingerprints_storage_colmajor Column-Major Order
   *
   * A class that is a model of the ColumnMajorFingerprintStorageConcept must support
   * the following operations:
   *
   * @code
   * std::string json = storage->header();
   * unsigned int n = storage->numBits();
   * unsigned int n = storage->numFingerprints();
   * Helium::Word *bit = storage->bit(index);
   * @endcode
   *
   * In the code above, storage is a pointer to an instance of a type that is a
   * model of the concept and index is an unsigned int. The index refers to the
   * bit in the fingerprint (e.g. in the range [0,1023] for a 1024-bit
   * fingerprint). The returned pointer should point to a memory location
   * containing num_fingerprints bits.
   *
   *
   */

   
   
  class RowMajorFingerprintOutputFile
  {
    public:
      RowMajorFingerprintOutputFile(const std::string &filename, unsigned int numBits) : m_file(filename)
      {
        m_numBytes = bitvec_num_words_for_bits(numBits) * sizeof(Word);
      }

      bool writeFingerprint(Word *fingerprint)
      {
        return m_file.write(fingerprint, m_numBytes);
      }

      bool writeHeader(const std::string &header)
      {
        return m_file.writeHeader(header);
      }

    private:
      BinaryOutputFile m_file;
      unsigned int m_numBytes;
  };

  class InMemoryRowMajorFingerprintStorage
  {
    public:
      InMemoryRowMajorFingerprintStorage(const std::string &filename)
      {
        TIMER("Loading InMemoryRowMajorFingerprintStorage:");
        load(filename);
      }

      ~InMemoryRowMajorFingerprintStorage()
      {
        delete [] m_fingerprints;
      }

      std::string header() const
      {
        return m_json;
      }

      unsigned int numBits() const
      {
        return m_numBits;
      }

      unsigned int numFingerprints() const
      {
        return m_numFingerprints;
      }

      Word* fingerprint(unsigned int index) const
      {
        return m_fingerprints + bitvec_num_words_for_bits(m_numBits) * index;
      }

    private:
      void load(const std::string &filename)
      {
        // open the file
        BinaryInputFile file(filename);
        if (!file) {
          throw std::runtime_error(make_string("Could not open fingerprint file ", filename));
          return;
        }

        // parse the JSON header
        m_json = file.header();
        Json::Reader reader;
        Json::Value data;
        if (!reader.parse(m_json, data)) {
          throw std::runtime_error(reader.getFormattedErrorMessages());
          return;
        }

        // make sure the required attributes are present
        if (!data.isMember("num_bits")) {
          throw std::runtime_error(make_string("JSON header for file ", filename, " does not contain 'num_bits' attribute"));
          return;
        }
        if (!data.isMember("num_fingerprints")) {
          throw std::runtime_error(make_string("JSON header for file ", filename, " does not contain 'num_fingerprints' attribute"));
          return;
        }

        // get attributes from header
        m_numBits = data["num_bits"].asUInt();
        m_numFingerprints = data["num_fingerprints"].asUInt();

        // allocate memory
        m_fingerprints = new Word[bitvec_num_words_for_bits(m_numBits) * m_numFingerprints];
        file.read(m_fingerprints, bitvec_num_words_for_bits(m_numBits) * m_numFingerprints * sizeof(Word));
      }

      std::string m_json; //!< JSON header
      Word *m_fingerprints;
      unsigned int m_numBits;
      unsigned int m_numFingerprints;
  };





  class FingerprintFile
  {
    public:
      FingerprintFile(const std::string &filename) : m_ifs(filename.c_str()), m_current(-1)
      {
        if (m_ifs)
          read32(m_ifs, m_numFingerprints);    
        m_numWords = 16;
      }

      unsigned int num_fingerprints() const
      {
        return m_numFingerprints;
      }

      unsigned int current() const
      {
        return m_current;
      }

      bool read_fingerprint(Word *fingerprint)
      {
        if (!m_ifs)
          return false;
        ++m_current;
        if (m_current == m_numFingerprints)
          return false;
        for (int i = 0; i < m_numWords; ++i)
          read64(m_ifs, fingerprint[i]);
        return m_ifs;
      }

    private:
      std::ifstream m_ifs;
      unsigned int m_numFingerprints;
      unsigned int m_current;
      int m_numWords;
  };

  struct InvertedFingerprintFileHeader
  {
    InvertedFingerprintFileHeader() : magic_number(get_magic_number())
    {
    }

    static unsigned int get_magic_number()
    {
      return 0x48650001;
    }

    unsigned int magic_number;
    unsigned int bits_per_word;
    unsigned int bits_per_fingerprint;
    unsigned int words_per_fingerprint;
    unsigned int words_per_fpbit;
    unsigned int num_fingerprints;
  };

  class InvertedFingerprintOutputFile
  {
    public:
      InvertedFingerprintOutputFile(unsigned int bitsPerFingerprint, unsigned int numFingerprints, const std::string &filename) 
          : m_ofs(filename.c_str(), std::ios_base::out | std::ios_base::binary), m_current(0)
      {
        // setup m_header
        m_header.bits_per_word = sizeof(Word) * 8; // e.g. 64
        m_header.bits_per_fingerprint = bitsPerFingerprint; // e.g. 1024
        m_header.words_per_fingerprint = m_header.bits_per_fingerprint / m_header.bits_per_word; // e.g. 16
        m_header.words_per_fpbit = (numFingerprints + numFingerprints % m_header.bits_per_word) / m_header.bits_per_word;
        m_header.num_fingerprints = numFingerprints;

        // write m_header
        if (!m_ofs)
          throw std::runtime_error(make_string("Could not open ", filename, " for writing."));
        
        // write the header
        m_ofs.write(reinterpret_cast<const char*>(&m_header), sizeof(InvertedFingerprintFileHeader));
        // allocate data
        m_data = new Word[m_header.words_per_fpbit * m_header.bits_per_fingerprint];
      }

      ~InvertedFingerprintOutputFile()
      {
        // write the data
        m_ofs.write(reinterpret_cast<const char*>(m_data), m_header.words_per_fpbit * m_header.bits_per_fingerprint * sizeof(Word));
        delete [] m_data;
      }

      void write(Word *fingerprint)
      {
        // check each bit in the fingerprint
        for (int i = 0; i < m_header.bits_per_fingerprint; ++i) {
          // skip this bit if it is not set
          if (!bitvec_get(i, fingerprint))
            continue;

          // set the correct bit
          bitvec_set(i * m_header.words_per_fpbit * 64 + m_current, m_data);
        }
        ++m_current;
      }

    private:
      InvertedFingerprintFileHeader m_header;
      std::ofstream m_ofs;
      unsigned int m_current;
      Word *m_data;
  };

  class InvertedFingerprintFile
  {
    public:
      InvertedFingerprintFile(const std::string &filename) : m_ifs(filename.c_str(), std::ios_base::in | std::ios_base::binary)
      {
        if (!m_ifs)
          throw std::runtime_error(make_string("Could not open ", filename, " for reading."));

        // read the header
        m_ifs.read(reinterpret_cast<char*>(&m_header), sizeof(InvertedFingerprintFileHeader));

        // check magic number
        if (m_header.magic_number != m_header.get_magic_number())
          throw std::runtime_error(make_string(filename, " is not an inverted fingerprint file."));

        // allocate memory
        m_data = new Word[m_header.words_per_fpbit];
      }

      ~InvertedFingerprintFile()
      {
        delete [] m_data;
      }

      unsigned int num_fingerprints() const
      {
        return m_header.num_fingerprints;
      }

      Word* allocate_result() const
      {
        return new Word[m_header.words_per_fpbit];
      }

      void search(Word *fingerprint, Word *result)
      {
        bool first = true;
        for (int i = 0; i < m_header.bits_per_fingerprint; ++i) { // foreach bit
          // skip this bit if it is not set
          if (!bitvec_get(i, fingerprint))
            continue;
        

          // compute offset for this fingerprint bit
          unsigned int offset = i * m_header.words_per_fpbit;
          
          m_ifs.seekg(sizeof(InvertedFingerprintFileHeader) + offset * sizeof(Word));
          m_ifs.read(reinterpret_cast<char*>(m_data), m_header.words_per_fpbit * sizeof(Word));

          if (first) {
            // if this is the first bit, just set result
            for (unsigned int j = 0; j < m_header.words_per_fpbit; ++j)
              result[j] = m_data[j];
            first = false;
          } else {
            // do set intersection
            for (unsigned int j = 0; j < m_header.words_per_fpbit; ++j)
              result[j] &= m_data[j];
          }
        }
      }

    private:
      InvertedFingerprintFileHeader m_header;
      std::ifstream m_ifs;
      Word *m_data;
  };

  class InvertedFingerprintFileCached
  {
    public:
      InvertedFingerprintFileCached(const std::string &filename)
      {
        // open the file
        std::ifstream ifs(filename.c_str(), std::ios_base::in | std::ios_base::binary);
        if (!ifs)
          throw std::runtime_error(make_string("Could not open ", filename, " for reading."));

        // read the header
        ifs.read(reinterpret_cast<char*>(&m_header), sizeof(InvertedFingerprintFileHeader));

        // check magic number
        if (m_header.magic_number != m_header.get_magic_number())
          throw std::runtime_error(make_string(filename, " is not an inverted fingerprint file."));

        // allocate memory
        m_data = new Word[m_header.words_per_fpbit * m_header.bits_per_fingerprint];
        // read the data
        ifs.read(reinterpret_cast<char*>(m_data), m_header.words_per_fpbit * m_header.bits_per_fingerprint * sizeof(Word));
      }

      ~InvertedFingerprintFileCached()
      {
        delete [] m_data;
      }

      unsigned int num_fingerprints() const
      {
        return m_header.num_fingerprints;
      }

      Word* allocate_fingerprint() const
      {
        return new Word[m_header.words_per_fingerprint];
      }
      
      Word* allocate_result() const
      {
        return new Word[m_header.words_per_fpbit];
      }

      void search(Word *fingerprint, Word *result)
      {
        bool first = true;
        for (int i = 0; i < m_header.bits_per_fingerprint; ++i) { // foreach bit
          // skip this bit if it is not set
          if (!bitvec_get(i, fingerprint))
            continue;

          // compute offset for this fingerprint bit
          unsigned int offset = i * m_header.words_per_fpbit;

          if (first) {
            // if this is the first bit, just set result
            for (unsigned int j = 0; j < m_header.words_per_fpbit; ++j)
              result[j] = m_data[offset + j];
            first = false;
          } else {
            // do set intersection
            for (unsigned int j = 0; j < m_header.words_per_fpbit; ++j)
              result[j] &= m_data[offset + j];
          }
        }
      }

    private:
      InvertedFingerprintFileHeader m_header;
      Word *m_data;
  };


  
}

#endif