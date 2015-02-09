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

#ifndef SDCI_COMMON_H_INCLUDED
#define SDCI_COMMON_H_INCLUDED

#include <cstddef>
#include <climits>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <iterator>

#include <stdint.h>

namespace sdci{
	namespace detail{

		typedef std::size_t size_type;

		typedef ::uint64_t uint64_type;

		class count_iterator
		: public std::iterator<std::output_iterator_tag, std::size_t>
		{
		public:
			typedef ::sdci::detail::size_type size_type;

			explicit count_iterator(size_type c = 0) : cnt(c) {}

			count_iterator& operator++ (){
				++cnt;
				return *this;
			}

			count_iterator operator++ (int){
				return count_iterator(cnt++);
			}

			count_iterator& operator* (){
				return *this;
			}

			const count_iterator& operator* () const{
				return *this;
			}

			count_iterator& operator= (size_type){
				return *this;
			}

			size_type count() const{
				return cnt;
			}

		private:
			size_type cnt;
		};

/*
#if __cplusplus >= 201103L
		typedef std::uint64_t uint64_type;
#elif UINT_MAX == 0xFFFFFFFFFFFFFFFFull
		typedef unsigned uint64_type;
#elif ULONG_MAX == 0xFFFFFFFFFFFFFFFFull
		typedef unsigned long uint64_type;
#elif ULLONG_MAX == 0xFFFFFFFFFFFFFFFFull
		typedef unsigned long long uint64_type;
#else
		#error You cannot use this library.
#endif
*/
		
		template <class Integer>
		inline Integer multiply_limited(Integer x, Integer y, Integer limit){
			if(y == 0){ return 0; }
			if(limit / y >= x){ return x * y; }
			return limit;
		}

		// setted least significant bit
		inline unsigned slsb64(uint64_type value){
#if !defined(SDCI_NO_USE_BUILTINS) && (defined(__GNUC__) || (defined(__clang__)))
			return __builtin_ctzll(value);
#else
			static const unsigned char table[64] = {
				0, 1, 2, 7, 3, 13, 8, 19, 4, 25, 14, 28, 9, 34, 20, 40,
				5, 17, 26, 38, 15, 46, 29, 48, 10, 31, 35, 54, 21, 50, 41, 57,
				63, 6, 12, 18, 24, 27, 33, 39, 16, 37, 45, 47, 30, 53, 49, 56,
				62, 11, 23, 32, 36, 44, 52, 55, 61, 22, 43, 51, 60, 42, 59, 58,
			};
			
			return table[(((value & -value) * 0x218A392CD3D5DBFull) & 0xFFFFFFFFFFFFFFFFull) >> 58];
#endif
		}

		inline unsigned smsb64(uint64_type value){
#if !defined(SDCI_NO_USE_BUILTINS) && (defined(__GNUC__) || (defined(__clang__)))
			return 63 - __builtin_clzll(value);
#else
			static const unsigned char table[256] = {
				0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
				4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
				5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
				5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
				6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
				6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
				6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
				6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
				7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
				7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
				7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
				7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
				7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
				7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
				7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
				7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
			};
			if(value & 0xffffffff00000000ull){
				if(value & 0xffff000000000000ull){
					if(value & 0xff00000000000000ull){
						return table[value >> 56 & 255] + 56;
					}
					else{
						return table[value >> 48 & 255] + 48;
					}
				}
				else{
					if(value & 0xff0000000000ull){
						return table[value >> 40 & 255] + 40;
					}
					else{
						return table[value >> 32 & 255] + 32;
					}
				}
			}
			else{
				if(value & 0xffff0000ull){
					if(value & 0xff000000ull){
						return table[value >> 24 & 255] + 24;
					}
					else{
						return table[value >> 16 & 255] + 16;
					}
				}
				else{
					if(value & 0xff00ull){
						return table[value >> 8 & 255] + 8;
					}
					else{
						return table[value >> 0 & 255] + 0;
					}
				}
			}
#endif
		}

		// ceil(log2(value))
		inline int ceillg64(uint64_type value){
			return value ? smsb64((value - 1) | 1) + 1 : 0;
		}

		inline void formaterr(){
			throw std::runtime_error("Format error");
		}
		
		inline void ioerr(){
			throw std::runtime_error("IO error");
		}
		
		template <class Tp>
		inline void read_data(std::istream& stream, Tp* value,
		               size_type size = sizeof(Tp)
		){
			stream.read(reinterpret_cast<char*>(value), size);
			if(stream.eof()){
				formaterr();
			}
			else if(stream.fail()){
				ioerr();
			}
		}

		template <class Tp>
		inline void read_vector(std::istream& stream, std::vector<Tp>& vec) try{
			size_type size = 0;
			read_data(stream, &size);
			vec.resize(size);
			if(size != 0){
				read_data(stream, &vec[0], size * sizeof(Tp));
			}
		}
		catch(...){
			vec.clear();
			throw;
		}

		template <>
		inline void read_vector(std::istream& stream, std::vector<bool> &vec){
			size_type size = 0;
			read_data(stream, &size);
			vec.resize(size);
			if(size > 0){
				std::vector<unsigned char> buf((size + 7) / 8);
				read_data(stream, &buf[0], buf.size());
				std::vector<bool>::iterator it = vec.begin();
				for(size_type i = 0; i < size; ++i){
					*it = ((buf[i / 8] >> (i % 8) & 1) != 0);
					++it;
				}
			}
		}
		
		template <class Tp>
		inline void write_data(std::ostream& stream, const Tp* value,
		                size_type size = sizeof(Tp)
		){
			stream.write(reinterpret_cast<const char*>(value), size);
		}
		
		template <class Tp>
		inline void write_vector(
			std::ostream& stream, const std::vector<Tp>& vec, size_type num_elements
		){
			num_elements = std::min<size_type>(num_elements, vec.size());
			write_data(stream, &num_elements);
			if(!vec.empty()){
				write_data(stream, &vec[0], num_elements * sizeof(Tp));
			}
		}

		template <class Tp>
		inline void write_vector(
			std::ostream& stream, const std::vector<Tp>& vec
		){
			write_vector(stream, vec, size_type(-1));
		}

		template <>
		inline void write_vector(
			std::ostream& stream, const std::vector<bool>& vec, size_type num_elements
		){
			num_elements = std::min<size_type>(num_elements, vec.size());
			write_data(stream, &num_elements);
			if(num_elements > 0){
				std::vector<unsigned char> buf((num_elements + 7) / 8);
				std::vector<bool>::const_iterator it = vec.begin();
				for(size_type i = 0; i < num_elements; ++i){
					if(*it){
						buf[i / 8] |= 1 << (i % 8);
					}
					++it;
				}
				write_data(stream, &buf[0], buf.size() * sizeof(buf[0]));
			}
		}

		template <>
		inline void write_vector(
			std::ostream& stream, const std::vector<bool>& vec
		){
			write_vector(stream, vec, size_type(-1));
		}
	}
}
#endif
