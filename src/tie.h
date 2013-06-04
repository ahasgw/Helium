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
#ifndef HELIUM_TIE_H
#define HELIUM_TIE_H

#include <utility>

namespace Helium {

  namespace impl {

    /**
     * Helper class for implementing tie() function. The tie function returns an
     * instance of this class which has an assignment operator taking a
     * std::pair as argument that assigns the pair values to the variables
     * passed in the constructor.
     */
    template<typename T1, typename T2>
    class tie_impl
    {
      public:
        /**
         * Constructor taking references to the variables.
         */
        tie_impl(T1 &first, T2 &second) : m_first(first), m_second(second)
        {
        }

        /**
         * Assignment operator for std::pair.
         */
        void operator=(const std::pair<T1, T2> &p)
        {
          // assign pair values to variable references
          m_first = p.first;
          m_second = p.second;
        }

      private:
        T1 &m_first; //!< First variable
        T2 &m_second; //!< Second variable
    };

  }

  /**
   * Helper function to easily assign std::pair values to two variables.
   *
   * @code
   * std::pair<int, int> foo()
   * {
   *   ...
   * }
   *
   * void bar()
   * {
   *   int a, b;
   *   TIE(a, b) = foo();
   *   ...
   * }
   * @endcode
   */
  template<typename T1, typename T2>
  impl::tie_impl<T1, T2> TIE(T1 &first, T2 &second)
  {
    return impl::tie_impl<T1, T2>(first, second);
  }

}

#endif
