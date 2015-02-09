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

#include "sampled_position_list.h"
#include <algorithm>
#include <utility>
#include <limits>

namespace sdci{
	namespace detail{
		const sampled_position_list::size_type sampled_position_list::npos;

		sampled_position_list::sampled_position_list
		(size_type entry_number, size_type reserved_node_size){
			initialize(entry_number, reserved_node_size);
		}

		void sampled_position_list::initialize
		(size_type entry_number, size_type reserved_node_size) try{
			num_nodes = 0;

			size_type lg_num_nodes = ::sdci::detail::ceillg64(reserved_node_size + 2);
			lfirst.change_params(lg_num_nodes, entry_number);
			lnext.change_params(lg_num_nodes, reserved_node_size);
		}
		catch(...){
			lfirst.clear();
			lnext.clear();
			throw;
		}

		void sampled_position_list::reserve(size_type reserved_node_size){
			if(reserved_node_size > num_nodes){
				size_type lg_num_nodes = ::sdci::detail::ceillg64(reserved_node_size + 2);
				if(lg_num_nodes > lfirst.bit_width()){
					lfirst.change_params(lg_num_nodes, lfirst.size());
					lnext.change_params(lg_num_nodes, reserved_node_size);
				}
				else if(lg_num_nodes == lnext.bit_width() && reserved_node_size > lnext.size()){ 
					lnext.change_params(lg_num_nodes, reserved_node_size);
				}
			}
		}

		void sampled_position_list::insert_first(size_type entry) try{
			if(num_nodes + 1 >= lnext.size()){
				size_type next_reserve_v = ::sdci::detail::multiply_limited<size_type>(num_nodes, 2, -3);
				size_type represent =
					(lnext.bit_width() == ::sdci::detail::packed_array::max_bit_width()
						? 1ull << lnext.bit_width()
						: -3ull
					);
				if(next_reserve_v > represent){
					next_reserve_v = ::sdci::detail::multiply_limited<size_type>(represent, 2, -3);
				}

				next_reserve_v = std::max<size_type>(next_reserve_v, 16);

				reserve(next_reserve_v);
			}

			const size_type val = lfirst.get(entry);
			lnext.set(num_nodes, val);
			lfirst.set(entry, ++num_nodes);
		}
		catch(...){
			lfirst.clear();
			lnext.clear();
			num_nodes = 0;
			throw;
		}

		sampled_position_list::size_type
		sampled_position_list::first_node(size_type entry) const{
			if(entry < lfirst.size()){
				return lfirst.get(entry) - 1;
			}
			return npos;
		}

		sampled_position_list::size_type
		sampled_position_list::next_node(size_type node_number) const{
			if(node_number < lnext.size()){
				return lnext.get(node_number) - 1;
			}
			return npos;
		}

		void sampled_position_list::shrink_to_fit() try{
			size_type lg_num_nodes = ::sdci::detail::ceillg64(num_nodes + 2);
			lfirst.change_params(lg_num_nodes, lfirst.size());
			lnext.change_params(lg_num_nodes, lnext.size());
		}
		catch(...){
			lfirst.clear();
			lnext.clear();
			num_nodes = 0;
		}

		void sampled_position_list::clear(){
			if(num_nodes > 0){
				lfirst.fill0();
//				lnext.fill0();
				num_nodes = 0;
			}
		}

		void sampled_position_list::save_stream(std::ostream &stream) const{
			::sdci::detail::write_data(stream, &num_nodes);
			lfirst.save_stream(stream);
			lnext.save_stream(stream, num_nodes);
		}

		void sampled_position_list::load_stream(std::istream &stream) try{
			::sdci::detail::read_data(stream, &num_nodes);
			lfirst.load_stream(stream);
			lnext.load_stream(stream);
		}
		catch(...){
			num_nodes = 0;
			lfirst.clear();
			lnext.clear();
			throw;
		}
	}
}


