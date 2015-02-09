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

#include "integer_set.h"
#include <limits>

namespace sdci{
	namespace detail{

		integer_set::integer_set(size_type new_size){
			initialize(new_size);
		}

#if __cplusplus >= 201103L
		integer_set::integer_set(integer_set&& other)
		: width(other.width), cnt(other.cnt),
		  buf(std::move(other.buf)), offset(std::move(other.offset))
		{
			other.width = 0;
			other.cnt = 0;
		}

		integer_set& integer_set::operator= (integer_set&& other){
			this->swap(other);
			return *this;
		}
#endif

		void integer_set::initialize(const size_type new_size) try{
			width = new_size;
			cnt = 0;
			size_type sum = calc_offset();
			buf.assign(sum, 0);
		} catch(...){
			width = 0;
			buf.clear();
			offset.clear();
			throw;
		}

		bool integer_set::insert(const value_type pos_signed){
			if(pos_signed + size_type() >= width + value_type() || contains(pos_signed)){
				return false;
			}
			size_type pos = pos_signed;
			++cnt;
			bool cont_flag = true;
			std::vector<size_type>::iterator level = offset.begin();
			while(cont_flag){
				data_type &bits = buf[*level + pos / value_width];
				++level;
				cont_flag = (level != offset.end()) && (bits == 0);
				bits |= data_type(1) << (pos % value_width);
				pos /= value_width;
			}
			return true;
		}

		bool integer_set::erase(const value_type pos_signed){
			if(contains(pos_signed)){
				size_type pos = pos_signed;
				--cnt;
				bool cont_flag = true;
				std::vector<size_type>::iterator level = offset.begin();
				while(cont_flag){
					data_type &bits = buf[*level + pos / value_width];
					data_type b = data_type(1) << (pos % value_width);
					++level;
					cont_flag = (level != offset.end()) && (bits == b);
					bits &= ~b;
					pos /= value_width;
				}
				return true;
			}
			return false;
		}

		void integer_set::clear(){
			if(cnt != 0){	
				std::fill(buf.begin(), buf.end(), data_type());
				cnt = 0;
			}
		}

		integer_set::value_type integer_set::successor(value_type pos_signed) const{
			if(pos_signed < 0){
				if(contains(0)){
					return 0;
				}
				pos_signed = 0;
			}
			size_type pos = pos_signed;
			if(pos >= width){
				return value_type(width);
			}
			std::vector<size_type>::const_iterator level = offset.begin();
			while(true){
				if(level == offset.end()){
					return value_type(width);
				}

				size_type next = pos / value_width;
				data_type bits = buf[*level + next];
				bits &= ~((data_type(2) << (pos % value_width)) - 1);
				if(bits != 0){
					pos = next * value_width + ::sdci::detail::slsb64(bits);
					break;
				}

				pos = next;
				++level;
			}
			while(level != offset.begin()){
				--level;
				data_type bits = buf[*level + pos];
				pos = pos * value_width + ::sdci::detail::slsb64(bits);
			}
			return value_type(pos);
		}

		integer_set::value_type integer_set::predecessor(value_type pos_signed) const{
			if(pos_signed < 0 || width == 0){
				return -1;
			}
			size_type pos = pos_signed;
			if(pos >= width){
				if(contains(width - 1)){
					return value_type(width - 1);
				}
				pos = width - 1;
			}
			std::vector<size_type>::const_iterator level = offset.begin();
			while(true){
				if(level == offset.end()){
					return -1;
				}

				size_type next = pos / value_width;
				data_type bits = buf[*level + next];
				bits &= (data_type(1) << (pos % value_width)) - 1;
				if(bits != 0){
					pos = next * value_width + ::sdci::detail::smsb64(bits);
					break;
				}

				pos = next;
				++level;
			}
			while(level != offset.begin()){
				--level;
				data_type bits = buf[*level + pos];
				pos = pos * value_width + ::sdci::detail::smsb64(bits);
			}
			return value_type(pos);
		}

		integer_set::size_type integer_set::calc_offset(){
			size_type sum = 0;
			offset.clear();
			if(width == 0){ return 0; }
			
			size_type new_size = width;
			do{
				offset.push_back(sum);
				new_size = (new_size + (value_width - 1)) / value_width;
				sum += new_size;
			} while(new_size > 1);
			return sum;
		}

		void integer_set::save_stream(std::ostream &stream) const{
			::sdci::detail::write_data(stream, &width);
			::sdci::detail::write_data(stream, &cnt);
			::sdci::detail::write_vector(stream, buf);
		}

		void integer_set::load_stream(std::istream &stream) try{
			::sdci::detail::read_data(stream, &width);
			::sdci::detail::read_data(stream, &cnt);
			::sdci::detail::read_vector(stream, buf);
			calc_offset();
		}
		catch(...){
			width = 0;
			cnt = 0;
			buf.clear();
			offset.clear();
			throw;
		}
	}
}

