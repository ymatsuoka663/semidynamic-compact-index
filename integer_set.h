/*
    Copyright (C) 2015, Yoshiaki Matsuoka


    This file is part of semidynamic-compact-index.

    semidynamic-compact-index is free software: you can redistribute it and/or 
    modify it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with semidynamic-compact-index. 
    If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SDCI_INTEGER_SET_H_INCLUDED
#define SDCI_INTEGER_SET_H_INCLUDED

#include "sdci_common.h"
#include <cstddef>
#include <vector>
#include <algorithm>
#include <utility>
#include <new>
#include <iostream>
#include <utility>

#if __cplusplus >= 201103L
#include <type_traits>
#endif

namespace sdci{
	namespace detail{

		class integer_set{
		public:
			typedef sdci::detail::size_type size_type;

#if __cplusplus >= 201103L
			typedef std::make_signed<size_type>::type value_type;
#else
			typedef std::ptrdiff_t value_type;
#endif

		private:
			typedef sdci::detail::uint64_type data_type;

			enum{ value_width = 64 };

		public:
			explicit integer_set(size_type new_size = 0);

#if __cplusplus >= 201103L
			integer_set(integer_set&& other);
			integer_set& operator= (const integer_set&) = default;
			integer_set& operator= (integer_set&& other);
			~integer_set() = default;
#endif

			void initialize(size_type new_size);
			bool contains(value_type pos) const;
			bool insert(value_type pos);
			bool erase(value_type pos);
			void clear();
			value_type successor(value_type pos) const;
			value_type predecessor(value_type pos) const;
			size_type limit() const;
			size_type size() const;
			void swap(integer_set& other);
			size_type heap_usage() const;
			void save_stream(std::ostream &stream) const;
			void load_stream(std::istream &stream);

#if 0
			void read_stream(std::istream& stream) try{
				impl::read_data(stream, &count);
				impl::read_vector(stream, buf);
			}
			catch(...){
				width = 0;
				count = 0;
				buf.clear();
				throw;
			}
			
			void write_stream(std::ostream& stream) const{
				impl::write_data(stream, &count);
				impl::write_vector(stream, buf);
			}
#endif
			
		private:
			size_type width;
			size_type cnt;
			std::vector<data_type> buf;
			std::vector<size_type> offset;
			
			size_type calc_offset();
#if 0
			static const value_type positive_infinity = std::numeric_limits<integer_set::value_type>::max_value();
			static const value_type negative_infinity = std::numeric_limits<integer_set::value_type>::min_value();
#endif

		};

		inline bool integer_set::contains(value_type pos) const{
			if(pos + size_type() >= width + value_type()){
				return false;
			}
			const size_type pos_unsig = pos;
			return (buf[pos_unsig / 64] >> (pos_unsig % 64)) & 1;
		}

		inline integer_set::size_type integer_set::limit() const{
			return width;
		}

		inline integer_set::size_type integer_set::size() const{
			return cnt;
		}

		inline void integer_set::swap(integer_set& other){
			std::swap(width, other.width);
			std::swap(cnt, other.cnt);
			buf.swap(other.buf);
			offset.swap(other.offset);
		}

		inline integer_set::size_type
		integer_set::heap_usage() const{
			return buf.capacity() * sizeof(buf[0]) + offset.capacity() * sizeof(offset[0]);
		}
	}
}


#endif

