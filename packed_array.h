/*
    Copyright (C) 2015, Yoshiaki Matsuoka


    This file is part of semidynamic-compact-index.

    semidynamic-compact-index is free software: you can redistribute it and/or 
    modify it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    semidynamic-compact-index is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with semidynamic-compact-index. 
    If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SDCI_PACKED_INTEGER_ARRAY_H_INCLUDED
#define SDCI_PACKED_INTEGER_ARRAY_H_INCLUDED

#include "sdci_common.h"
#include <cstddef>
#include <vector>
#include <algorithm>
#include <utility>
#include <iostream>

namespace sdci{
	namespace detail{
		class packed_array{
		public:
			typedef ::sdci::detail::size_type size_type;
			typedef ::sdci::detail::uint64_type value_type;
		private:
			enum{ value_width = 64 };
			
		public:
			explicit packed_array(size_type bit_width = 0, size_type size = 0);
			size_type size() const;
			size_type bit_width() const;
			static size_type max_bit_width();
			void swap(packed_array& other);
			void change_params(size_type new_bit_width, size_type new_size);
			value_type get(size_type pos) const;
			void set(size_type pos_, value_type val);
			void shrink_to_fit();
			void clear();
			void fill0();
			void fill1();
			size_type heap_usage() const;
			void save_stream(std::ostream &stream, size_type save_size = size_type(-1)) const;
			void load_stream(std::istream &stream);

#if __cplusplus >= 201103L
			packed_array(const packed_array&) = default;
			packed_array(packed_array &&) = default;
			packed_array& operator= (const packed_array &) = default;
			packed_array& operator= (packed_array &&) = default;
			~packed_array() = default;
#endif

		private:
			size_type bwidth;
			size_type len;
			std::vector<value_type> buf;

			static size_type get_necessary_size(size_type bit_width, size_type size);
		
		};
	

		// inline functions

		inline packed_array::size_type
		packed_array::size()
		const{
			return len;
		}
		
		inline packed_array::size_type
		packed_array::bit_width()
		const{
			return bwidth;
		}
		
		inline packed_array::size_type
		packed_array::max_bit_width()
		{
			return value_width;
		}
		
		inline void
		packed_array::swap(packed_array &other){
			std::swap(bwidth, other.bwidth);
			std::swap(len, other.len);
			buf.swap(other.buf);
		}
		
		inline void
		packed_array::clear(){
			std::vector<value_type>().swap(buf);
			len = 0;
		}

		inline void
		packed_array::fill0(){
			std::fill(buf.begin(), buf.end(), value_type());
		}

		inline void
		packed_array::fill1(){
			std::fill(buf.begin(), buf.end(), value_type(-1));
		}

		inline packed_array::size_type
		packed_array::heap_usage() const{
			return buf.capacity() * sizeof(buf[0]);
		}
	}
}

#endif

