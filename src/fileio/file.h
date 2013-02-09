#ifndef HELIUM_FILEIO_FILE_H
#define HELIUM_FILEIO_FILE_H

#include "../util.h"

#include <fstream>

namespace Helium {

  /**
   * @page binary_files Helium Binary Files
   *
   * The exact format is described below but for using these file
   * formats it is enough to know that all files contain a JSON header
   * followed by the binary data itself.
   *
   * @section binary_files_format The Binary File Format
   *
   * The first 4 bytes are a magic number (0x48650001). The next 4 bytes contain
   * the length (in bytes) of the JSON header including zero termination
   * character. The JSON header starts at position 8 and ends at 8 plus the
   * number of bytes contained in the second 4 byte field. What follows after
   * the JSON header is binary data and the format depends on the specific file
   * format.
   */

  /**
   * @brief Helper class for all Helium binary input file formats.
   *
   * The HeliumInputFile class is a helper class used for all Helium binary
   * input file formats.
   */
  class BinaryInputFile
  {
    public:
      enum Error
      {
        /**
         * No error, the file object is valid.
         */
        NoError,
        /**
         * Could not open the file (e.g. file does not exist).
         */
        CouldNotOpen,
        /**
         * Magic number does not match (e.g. file is not a Helium binary file
         * or the file got corrupted).
         */
        InvalidMagic,
        /**
         * The JSON header could not be read from the file.
         */
        InvalidHeader
      };

      BinaryInputFile() : m_error(NoError)
      {
      }

      BinaryInputFile(const std::string &filename) : m_error(NoError)
      {
        open(filename);
      }

      /**
       * Open the file with the specified filename. This method also checks the
       * file's magic number and reads the JSON header. If this method returns
       * false the error() method can be used to get the type of error that
       * occured.
       *
       * @param filename The filename for the file to open.
       *
       * @return True if the file was opened successfully, the magic number is
       *         valid and the JSON header was read successfully.
       */
      bool open(const std::string &filename);

      /**
       * Close the file.
       */
      void close()
      {
        m_error = NoError;
        m_json.clear();
        m_ifs.close();
      }

      /**
       * Get the error flag.
       *
       * @return The error flag.
       */
      Error error() const
      {
        return m_error;
      }

      /**
       * Convert to bool, can be used to check if stream is valid.
       *
       * @return True if the file stream is valid.
       */
      operator bool() const
      {
        return (bool)m_ifs;
      }

      /**
       * Get the current file stream position. The binary data starts at
       * position 0.
       *
       * @return The file stream position.
       */
      std::ios_base::streampos tell()
      {
        return m_ifs.tellg() - m_offset;
      }

      /**
       * Set the file stream position.
       *
       * @param pos The new file stream position.
       */
      bool seek(std::ios_base::streampos pos)
      {
        return (bool)m_ifs.seekg(pos + m_offset);
      }

      /*
      bool seekg(std::ios_base::streamoff offset, std::ios_base::seekdir dir)
      {
        if (dir == std::ios_base::beg)
          return m_ifs.seekg(off + m_offset, dir);
        else
          return m_ifs.seekg(off, dir);
      }
      */

      /**
       * Get the JSON header.
       *
       * @return The JSON header.
       */
      std::string header() const
      {
        return m_json;
      }

      /**
       * Read data from the file.
       *
       * @param data The data to read from the file.
       * @param size The size of the data to read in bytes.
       *
       * @return True if the data was read successfully.
       */
      bool read(char *data, std::size_t size)
      {
        return (bool)m_ifs.read(data, size);
      }

      template<typename T>
      bool read(T *data, std::size_t size)
      {
        return (bool)m_ifs.read(reinterpret_cast<char*>(data), size);
      }

    private:
      std::ifstream m_ifs; //!< File handle
      std::string m_json; //!< JSON header
      std::ios_base::streampos m_offset; //!< Offset where binary data starts
      Error m_error; //!< Error flag
  };

  /**
   * @brief Helper class for all Helium binary output file formats.
   *
   * The HeliumOutputFile class is a helper class used for all Helium binary
   * output file formats.
   */
  class BinaryOutputFile
  {
    public:
      enum Error
      {
        /**
         * No error, the file object is valid.
         */
        NoError,
        /**
         * Could not open the file (e.g. file does not exist).
         */
        CouldNotOpen
      };

      BinaryOutputFile() : m_error(NoError)
      {
      }

      BinaryOutputFile(const std::string &filename) : m_error(NoError)
      {
        open(filename);
      }

      /**
       * Open the file with the specified filename.
       *
       * @param filename The filename for the file to open.
       *
       * @return True if the file was opened successfully.
       */
      bool open(const std::string &filename);

      /**
       * Close the file.
       */
      void close()
      {
        m_error = NoError;
        m_ofs.close();
      }

      /**
       * Get the error flag.
       *
       * @return The error flag.
       */
      Error error() const
      {
        return m_error;
      }

      /**
       * Convert to bool, can be used to check if stream is valid.
       *
       * @return True if the file stream is valid.
       */
      operator bool() const
      {
        return (bool)m_ofs;
      }

      /**
       * Get the current file stream position. The binary data starts at
       * position 0.
       *
       * @return The file stream position.
       */
      std::ios_base::streampos tell()
      {
        return m_ofs.tellp() - m_offset;
      }

      /**
       * Set the file stream position.
       *
       * @param pos The new file stream position.
       */
      bool seek(std::ios_base::streampos pos)
      {
        return (bool)m_ofs.seekp(pos + m_offset);
      }

      /**
       * Write data to the file.
       *
       * @param data The data to write to the file.
       * @param size The size of the data to write in bytes.
       *
       * @return True if the data was written successfully.
       */
      bool write(const char *data, std::size_t size)
      {
        return (bool)m_ofs.write(data, size);
      }

      template<typename T>
      bool write(const T *data, std::size_t size)
      {
        return (bool)m_ofs.write(reinterpret_cast<const char*>(data), size);
      }


      /**
       * Write the JSON header to the file.
       *
       * @param header The JSON header.
       *
       * @return true if the header was written successfully.
       */
      bool writeHeader(const std::string &header);

    private:
      std::ofstream m_ofs; //!< File handle
      std::ios_base::streampos m_offset; //!< Offset where binary data starts
      Error m_error; //!< Error flag
  };

}

#endif
