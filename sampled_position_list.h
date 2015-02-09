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

#ifndef SDCI_SAMPLED_POSITION_LIST_H_INCLUDED
#define SDCI_SAMPLED_POSITION_LIST_H_INCLUDED

#include "sdci_common.h"
#include <cstddef>
#include <climits>
#include <stdexcept>
#include <iostream>
#include "packed_array.h"

namespace sdci{
	namespace detail{
		class sampled_position_list {
		public:
			typedef ::sdci::detail::size_type size_type;
			typedef ::sdci::detail::size_type value_type;
			
			const static size_type npos = size_type(-1);
			
			explicit sampled_position_list(size_type entry_number = 0, size_type reserved_node_size = 0);
			void initialize(size_type entry_number, size_type reserved_node_size = 0);
			void reserve(size_type size);
			void insert_first(size_type entry);
			size_type first_node(size_type entry) const;
			size_type next_node(size_type node_number) const;
			void swap(sampled_position_list &other);
			size_type entry_size() const;
			size_type node_size() const;
			void shrink_to_fit();
			void clear();
			size_type heap_usage() const;
			void save_stream(std::ostream &stream) const;
			void load_stream(std::istream &stream);

#if __cplusplus >= 201103L
			sampled_position_list(const sampled_position_list &) = default;
			sampled_position_list(sampled_position_list &&) = default;
			sampled_position_list& operator= (const sampled_position_list &) = default;
			sampled_position_list& operator= (sampled_position_list &&) = default;
			~sampled_position_list() = default;
#endif
			
		private:
			size_type num_nodes;
			::sdci::detail::packed_array lfirst;
			::sdci::detail::packed_array lnext;
		};
		
		//inline functions
		inline sampled_position_list::size_type
		sampled_position_list::entry_size()
		const{
			return lfirst.size();
		}
		
		inline sampled_position_list::size_type
		sampled_position_list::node_size()
		const{
			return num_nodes;
		}
		
		inline void
		sampled_position_list::swap(sampled_position_list &other){
			std::swap(this->num_nodes, other.num_nodes);
			lfirst.swap(other.lfirst);
			lnext.swap(other.lnext);
		}

		inline sampled_position_list::size_type
		sampled_position_list::heap_usage() const{
			return lfirst.heap_usage() + lnext.heap_usage();
		}
	}
}
#endif

