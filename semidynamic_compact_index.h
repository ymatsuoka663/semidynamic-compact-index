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

#ifndef SDCI_SEMIDYNAMIC_COMPACT_INDEX_H_INCLUDED
#define SDCI_SEMIDYNAMIC_COMPACT_INDEX_H_INCLUDED

#include "sdci_common.h"
#include <cstddef>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <utility>
#include <climits>
#include <iostream>
#include <cstring>
#include <fstream>
#include <limits>

#include "integer_set.h"
#include "sampled_position_list.h"
#include "packed_array.h"

namespace sdci{

	class semidynamic_compact_index{
	public:
		typedef ::sdci::detail::size_type size_type;

		/*
			Default constructor.
		*/
		semidynamic_compact_index();

		/*
			Sets parameters and create index of empty text.

			Parameters
			- sigma: The alphabet size.
			- param_q: The parameter q.
			- param_k: The parameter k.

			Preconditions
			- 1 <= param_k <= param_q
			- (sigma^param_q * 8) must be representable in size_type.
		*/
		semidynamic_compact_index(
			size_type sigma, size_type param_q, size_type param_k
		);

		/*
			Changes parameters and create index of empty text.
			After calling this function,
			the length of text will be 0.
			For each argument, if it equals (size_type(-1)),
			then the corresponding parameter does not be changed.

			Parameters
			- sigma: The alphabet size.
			- param_q: The parameter q.
			- param_k: The parameter k.

			Preconditions
			- 1 <= param_k <= param_q
			- (sigma^param_q * 8) must be representable in size_type.
		*/
		void initialize(
			size_type sigma, size_type param_q, size_type param_k
		);

		void reserve(size_type expected_max_text_length);

		/*
			Assigns characters as the text.

			Parameters
			- first, last: Input iterators to the initial and final positions of the appending characters. The range used is [first, last).

			Precondition
			- The values in [first, last) must be less than alphabet_size and must not be negative.

			Note
			- Calling this function is equivalent to calling clear() and append(first,last). 
		*/
		template <class InputIterator>
		void assign(InputIterator first, InputIterator last);

		/*
			Appends characters after the current text.

			Parameters
			- first, last: Input iterators to the initial and final positions of the appending characters. The range used is [first, last).

			Preconditions
			- The values in [first, last) must be less than alphabet_size and must not be negative.
		*/
		template <class InputIterator>
		void append(InputIterator first, InputIterator last);

		/*
			Sets the length of text to 0.
		*/
		void clear();

		void swap(semidynamic_compact_index &other);

		/*
			Returns the alphabet size (i.e. sigma).
		*/
		size_type alphabet_size() const;

		/*
			Returns the parameter q.
		*/
		size_type param_q() const;

		/*
			Returns the parameter k.
		*/
		size_type param_k() const;

		/*
			Returns the length of text.
		*/
		size_type text_length() const;

		/*
			Returns the length of text.
			This is the same as text_length().
		*/
		size_type text_size() const;

		/*
			Returns the maximum length of pattens that this index allows,
			i.e. q-k+1.
		*/
		size_type max_pattern_length() const;

		/*
			Returns the maximum length of pattens that this index allows.
			This is the same as max_pattern_length().
		*/
		size_type max_pattern_size() const;

		/*
			Computes the usage of heap.
		*/
		size_type heap_usage() const;

		/*
			Computes the usage of memory.
			It is equivalent to (heap_usage() + sizeof(*this)).
		*/
		size_type memory_usage() const;

		/*
			Computes all occurrences of given pattern and writes to occ_result.

			Parameters
			- pattern_first, pattern_last: Input iterators to the initial and final positions of given pattern. The range used is [pattern_first, pattern_last).
			- occ_result: Output iterator to the initial position of the range where the occurrences of given pattern are stored.

			Preconditions
			- The length of pattern must not greater than max_pattern_length (i.e. q-k+1).

			Return Value
			- Let r be the return value. Then the occurrences are writtern in range [occ_result, r).

			Hint
			- std::vector and std::back_inserter may be useful for occ_result.
		*/
		template <class InputIterator, class OutputIterator>
		OutputIterator locate(
			InputIterator pattern_first, InputIterator pattern_last,
			OutputIterator occ_result
		) const;

		/*
			Computes the number of all occurrences of given pattern.

			Parameters
			- pattern_first, pattern_last: Input iterators to the initial and final positions of given pattern. The range used is [pattern_first, pattern_last).

			Precondition
			- The length of pattern must not greater than max_pattern_length (i.e. q-k+1).

			Return Value
			- The number of all occurrences of given pattern.

			Note
			- This function requires as the same time as locating.
		*/
		template <class InputIterator>
		size_type count(
			InputIterator pattern_first, InputIterator pattern_last
		) const;

		/*
			Retrieves the current text.

			Parameter
			- output_itr: Forward iterator to the initial position of the range where the text are stored.

			Return Value
			- Let r be the return value. Then the text are writtern in range [output_itr, r).

			Complexity
			- Let n be the length or current text. If output_itr is a random access iterator, then this function takes O(n+sigma^q) time. Otherwise, this function may take more time.
		*/
		template <class ForwardIterator>
		ForwardIterator retrieve(ForwardIterator output_itr) const;

		/*
			Extracts text[from_ext..from_ext+length-1].

			Parameter
			- from_ext: Starting position of extracted string (0-based).
			- length_ext: Length of extracted string.
			- output_itr: Forward iterator to the initial position of the range where the text are stored.

			Return Value
			- Let r be the return value. Then the extracted text are writtern in range [output_itr, r).

			Complexity
			- Let n be the length or current text. If output_itr is a random access iterator, then this function takes O(n+sigma^q) time. Otherwise, this function may take more time.

			Note
			- The length of extracted string can be less than length_ext when from_ext+length is greater than n.
			- This function may take much time even if length_ext is small.
		*/
		template <class ForwardIterator>
		ForwardIterator extract(size_type from, size_type length, ForwardIterator output) const;

		void save_file(const char *filename) const;
		void save_stream(std::ostream &stream) const;
		void load_file(const char *filename);
		void load_stream(std::istream &stream);

#if __cplusplus >= 201103L
		semidynamic_compact_index(const semidynamic_compact_index &) = default;
		semidynamic_compact_index(semidynamic_compact_index&&);
		semidynamic_compact_index& operator= (const semidynamic_compact_index &) = default;
		semidynamic_compact_index& operator= (semidynamic_compact_index&& other);
		~semidynamic_compact_index() = default;
#endif


	private:
		typedef ::sdci::detail::uint64_type encode_type;

		encode_type lshift(encode_type, size_type) const;
		encode_type rshift(encode_type, size_type) const;
		encode_type mask(encode_type, size_type) const;

		template <class InputIterator>
		void reserve_if_able(InputIterator, InputIterator, std::input_iterator_tag);

		template <class InputIterator>
		void reserve_if_able(InputIterator first, InputIterator last, std::forward_iterator_tag);

		template <class OutputIterator>
		OutputIterator locate_dfs(encode_type pattern, size_type offset, OutputIterator result) const;

		static size_type calc_node_width(size_type param_q, size_type param_k, size_type size);

		void invalidarg(encode_type value) const;
		void ptnlenerr() const;

		size_type m_sigma;
		size_type m_param_q;
		size_type m_param_k;
		size_type m_textlen;
		encode_type m_last_qgram;
		size_type m_next_sampling_pos;
		bool m_first_appearance;
		std::vector<encode_type> m_pow_sigma;

		::sdci::detail::sampled_position_list m_list_sampled;
		::sdci::detail::packed_array m_efirst, m_enext;
		::sdci::detail::integer_set m_encQ;
	};
}

#include "sdci_impl.h"

#endif

