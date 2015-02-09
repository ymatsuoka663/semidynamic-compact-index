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

#include "packed_array.h"
#include <stdexcept>
#include <limits>

namespace sdci{
	namespace detail{
		packed_array::packed_array
		(size_type bit_width_, size_type size_)
		: bwidth(bit_width_), len(size_), buf(get_necessary_size(bit_width_, size_))
		{
		}

		packed_array::value_type
		packed_array::get
		(size_type pos_) const{
			if(bwidth == value_width){
				return buf[pos_];
			}
			value_type ret = 0;
			size_type bit_pos = bwidth * pos_;
			size_type bit_div = bit_pos / value_width;
			size_type bit_mod = bit_pos % value_width;
			ret = buf[bit_div] >> bit_mod;
			if(value_width - bit_mod < bwidth){
				ret |= buf[bit_div + 1] << (value_width - bit_mod);
			}
			return ret & ((static_cast<value_type>(1) << bwidth) - 1);
		}

		void
		packed_array::set
		(size_type pos_, value_type val_){
			if(bwidth == value_width){
				buf[pos_] = val_;
			}
			else{
				size_type bit_pos = bwidth * pos_;
				size_type bit_div = bit_pos / value_width;
				size_type bit_mod = bit_pos % value_width;
				value_type mask = (static_cast<value_type>(1) << bwidth) - 1;
				val_ &= mask;
				
				buf[bit_div] = (buf[bit_div] & ~(mask << bit_mod)) | (val_ << bit_mod);
				size_type rest = value_width - bit_mod;
				if(rest < bwidth){
					buf[bit_div + 1] = (buf[bit_div + 1] & ~(mask >> rest)) | (val_ >> rest);
				}
			}
		}

		void
		packed_array::change_params
		(size_type new_bit_width_, size_type new_size_){
			if(new_bit_width_ == 0 || new_size_ == 0){
				buf.clear();
				bwidth = new_bit_width_;
				len = 0;
			}
			else if(bwidth == new_bit_width_){
				buf.resize(get_necessary_size(new_bit_width_, new_size_));
				len = new_size_;
			}
			else{
				packed_array new_pa(new_bit_width_, new_size_);
				for(size_type i = std::min(len, new_size_); i--; ){
					new_pa.set(i, this->get(i));
				}
				this->swap(new_pa);
			}
		}
		
		packed_array::size_type
		packed_array::get_necessary_size
		(size_type bit_width_, size_type size_){
			if(bit_width_ == 0){
				return 0;
			}
			if(bit_width_ > value_width){
				throw std::invalid_argument("packed_array::get_necessary_size");
			}
			if((std::numeric_limits<value_type>::max() - value_width + 1) / bit_width_ >= size_){
				return (bit_width_ * size_ + value_width - 1) / value_width;
			}
			throw std::overflow_error("packed_array::get_necessary_size");
		}

		void packed_array::shrink_to_fit(){
			if(buf.size() != buf.capacity()){
				std::vector<value_type>(buf).swap(buf);
			}
		}

		void packed_array::save_stream(std::ostream &stream, size_type save_size) const{
			save_size = std::min(save_size, len);
			size_type num_write = (save_size * bwidth + value_width - 1) / value_width;
		
			::sdci::detail::write_data(stream, &bwidth);
			::sdci::detail::write_data(stream, &save_size);
			::sdci::detail::write_vector(stream, buf, num_write);
		}

		void packed_array::load_stream(std::istream &stream) try{
			::sdci::detail::read_data(stream, &bwidth);
			::sdci::detail::read_data(stream, &len);
			::sdci::detail::read_vector(stream, buf);
		}
		catch(...){
			bwidth = 0;
			len = 0;
			buf.clear();
			throw;
		}
	}
}
