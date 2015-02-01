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

#if defined(SDCI_SEMIDYNAMIC_COMPACT_INDEX_H_INCLUDED) && \
    !defined(SDCI_SDCI_IMPL_H_INCLUDED)
#define SDCI_SDCI_IMPL_H_INCLUDED

namespace sdci{
	inline semidynamic_compact_index::encode_type 
	semidynamic_compact_index::lshift(encode_type value, size_type ch_len) const{
		return value * m_pow_sigma[ch_len];
	}
	
	inline semidynamic_compact_index::encode_type 
	semidynamic_compact_index::rshift(encode_type value, size_type ch_len) const{
		return value / m_pow_sigma[ch_len];
	}
	
	inline semidynamic_compact_index::encode_type
	semidynamic_compact_index::mask(encode_type value, size_type ch_len) const{
		return value % m_pow_sigma[ch_len];
	}

	inline semidynamic_compact_index::size_type
	semidynamic_compact_index::alphabet_size() const{
		return m_sigma;
	}
	
	inline semidynamic_compact_index::size_type
	semidynamic_compact_index::text_size() const{
		return m_textlen;
	}

	inline semidynamic_compact_index::size_type
	semidynamic_compact_index::text_length() const{
		return text_size();
	}
	
	inline semidynamic_compact_index::size_type
	semidynamic_compact_index::max_pattern_size() const{
		return m_param_q - m_param_k + 1;
	}

	inline semidynamic_compact_index::size_type
	semidynamic_compact_index::max_pattern_length() const{
		return max_pattern_size();
	}

	inline semidynamic_compact_index::size_type
	semidynamic_compact_index::param_q() const{
		return m_param_q;
	}

	inline semidynamic_compact_index::size_type
	semidynamic_compact_index::param_k() const{
		return m_param_k;
	}

	inline semidynamic_compact_index::size_type
	semidynamic_compact_index::heap_usage() const{
		return
			m_list_sampled.heap_usage() + m_efirst.heap_usage() +
			m_enext.heap_usage() + m_encQ.heap_usage() +
			m_pow_sigma.capacity() * sizeof(m_pow_sigma[0]);
	}

	inline semidynamic_compact_index::size_type
	semidynamic_compact_index::memory_usage() const{
		return heap_usage() + sizeof(*this);
	}

	template <class InputIterator>
	void semidynamic_compact_index::reserve_if_able
	(InputIterator, InputIterator, std::input_iterator_tag){
	}
	
	template <class ForwardIterator>
	void semidynamic_compact_index::reserve_if_able
	(ForwardIterator first, ForwardIterator last, std::forward_iterator_tag){
		reserve(m_textlen + std::distance(first, last));
	}

	template <class InputIterator>
	void semidynamic_compact_index::assign
	(InputIterator first, InputIterator last){
		clear();
		append(first, last);
	}

	template <class InputIterator>
	void semidynamic_compact_index::append
	(InputIterator first, InputIterator last){
		if(first == last){
			return;
		}
		if(m_sigma == 0){
			throw std::runtime_error("semidynamic_compact_index::append");
		}

		typedef typename std::iterator_traits<InputIterator>::iterator_category category;
		reserve_if_able(first, last, category());

		for(; first != last; ++first){
			const encode_type next_ch = static_cast<encode_type>(*first);
			if(next_ch >= m_sigma){
				invalidarg(next_ch);
			}
			const encode_type next_qgram =
				lshift(mask(m_last_qgram, m_param_q - 1), 1) + next_ch;
			++m_textlen;

			if(m_textlen >= m_param_q){
				if(m_textlen == m_next_sampling_pos){
					m_list_sampled.insert_first(next_qgram);
					m_next_sampling_pos += m_param_k;
				}

				if(m_first_appearance){
					m_enext.set(m_last_qgram, m_efirst.get(next_qgram));
					m_efirst.set(next_qgram, rshift(m_last_qgram, m_param_q - 1) + 1);
				}
				m_first_appearance = m_encQ.insert(next_qgram);
			}
			m_last_qgram = next_qgram;
		}
	}

	template <class InputIterator, class OutputIterator>
	OutputIterator semidynamic_compact_index::locate
	(InputIterator first, InputIterator last, OutputIterator result) const
	{
		if(first == last){
			return result;
		}

		encode_type ptn_enc = 0;
		size_type ptn_len = 0;
		for(; first != last; ++first){
			const encode_type next = static_cast<encode_type>(*first);
			if(next >= m_sigma){
				return result;
			}

			ptn_enc = lshift(ptn_enc, 1) + next;
			++ptn_len;
			if(ptn_len > max_pattern_size()){
				ptnlenerr();
			}
		}

		if(ptn_len > m_textlen){
			return result;
		}

		if(m_textlen < m_param_q){
			const size_type num_cand = m_textlen - ptn_len;
			for(size_t i = 0; i <= num_cand; ++i){
				if(mask(rshift(m_last_qgram, num_cand - i), ptn_len) == ptn_enc){
					*result = i;
					++result;
				}
			}
			return result;
		}

		const size_type difflen = m_param_q - ptn_len;
		const encode_type ptn_first = lshift(ptn_enc, difflen);
		const encode_type ptn_last = lshift(ptn_enc + 1, difflen);

		typedef ::sdci::detail::integer_set::value_type signed_enc_type;
		for(signed_enc_type p = m_encQ.successor(static_cast<signed_enc_type>(ptn_first) - 1);
			static_cast<encode_type>(p) < ptn_last;
			p = m_encQ.successor(p)
		){
			result = locate_dfs(p, 0, result);
		}

		const size_type covered = ((m_textlen - m_param_q) / m_param_k + 1) * m_param_k;
		const size_type offset = m_textlen - m_param_q;
		for(size_type i = 1; i <= difflen; ++i){
			if(mask(rshift(m_last_qgram, difflen - i), ptn_len) == ptn_enc){
				if(i + offset >= covered){
					*result = i + offset;
					++result;
				}
				else if(m_first_appearance){
					result = locate_dfs(m_last_qgram, i, result);
				}
			}
		}

		return result;
	}

	template <class OutputIterator>
	OutputIterator semidynamic_compact_index::locate_dfs
	(encode_type ptn, size_type offset, OutputIterator result) const
	{
		typedef ::sdci::detail::sampled_position_list::value_type list_value_type;
		for(list_value_type nd = m_list_sampled.first_node(ptn);
			nd != ::sdci::detail::sampled_position_list::npos;
			nd = m_list_sampled.next_node(nd)
		){

			size_type pos = nd * m_param_k + offset;
			*result = pos;
			++result;
		}
		
		if(offset < m_param_k - 1){
			encode_type eattr = m_efirst.get(ptn);
			const encode_type rsptn = rshift(ptn, 1);
			while(eattr != 0){
				const encode_type nextptn = rsptn + lshift(eattr - 1, m_param_q - 1);
				result = locate_dfs(nextptn, offset + 1, result);
				eattr = m_enext.get(nextptn);
			}
		}
		
		return result;
	}

	template <class InputIterator>
	semidynamic_compact_index::size_type
	semidynamic_compact_index::count
	(InputIterator first, InputIterator last) const
	{
		return locate(first, last, ::sdci::detail::count_iterator()).count();
	}

	template <class ForwardIterator>
	ForwardIterator
	semidynamic_compact_index::retrieve(ForwardIterator output) const{
		ForwardIterator retval = output;
		advance(retval, m_textlen);

		if(m_textlen < m_param_q){
			for(size_type i = 0; i < m_textlen; ++i){
				*output = mask(rshift(m_last_qgram, m_textlen - i - 1), 1);
				++output;
			}
		}
		else{
			const size_type covered = ((m_textlen - m_param_q) / m_param_k + 1) * m_param_k;
			ForwardIterator it = output;
			advance(it, covered);
			for(size_type i = 0; i < m_textlen - covered; ++i){
				*it = mask(rshift(m_last_qgram, m_textlen - covered - i - 1), 1);
				++it;
			}

			const size_type kinds_of_qgrams = m_pow_sigma.back();
			for(size_type w = m_encQ.successor(-1); w < kinds_of_qgrams; w = m_encQ.successor(w)){
				typedef ::sdci::detail::sampled_position_list::value_type list_value_type;
				for(list_value_type nd = m_list_sampled.first_node(w);
					nd != ::sdci::detail::sampled_position_list::npos;
					nd = m_list_sampled.next_node(nd)
				){
					it = output;
					advance(it, m_param_k * nd);
					for(size_type j = 0; j < m_param_k; ++j){
						*it = mask(rshift(w, m_param_q - j - 1), 1);
						++it;
					}
				}
			}
		}

		return retval;
	}

	template <class ForwardIterator>
	ForwardIterator
	semidynamic_compact_index::extract
	(size_type from, size_type length, ForwardIterator output) const{
		if(length == 0 || from >= m_textlen){
			return output;
		}
		if(length > m_textlen - from){
			length = m_textlen - from;
		}

		ForwardIterator retval = output;
		advance(retval, length);

		size_type remain = length;

		if(m_textlen < m_param_q){
			for(size_type i = 0; i < length; ++i){
				*output = mask(rshift(m_last_qgram, m_textlen - i - from - 1), 1);
				++output;
				--remain;
			}
		}
		else{
			const size_type covered = ((m_textlen - m_param_q) / m_param_k + 1) * m_param_k;
			if(from + length > covered){
				const size_type dif = from + length - covered;
				const size_type uncovered = m_textlen - covered;
				ForwardIterator it = output;
				size_type i = 0;
				if(from < covered){
					advance(it, covered - from);
				}
				else{
					i = from - covered;
				}
				for(; i < dif; ++i){
					*it = mask(rshift(m_last_qgram, uncovered - i - 1), 1);
					++it;
					--remain;
				}
			}

			if(remain == 0){
				return retval;
			}
			const size_type kinds_of_qgrams = m_pow_sigma.back();
			for(size_type w = m_encQ.successor(-1); w < kinds_of_qgrams; w = m_encQ.successor(w)){
				typedef ::sdci::detail::sampled_position_list::value_type list_value_type;
				for(list_value_type nd = m_list_sampled.first_node(w);
					nd != ::sdci::detail::sampled_position_list::npos;
					nd = m_list_sampled.next_node(nd)
				){
					const size_type spos = m_param_k * nd;
					if(spos + m_param_k <= from){
						break;
						// We assume that the sampled positions of w are in descending order.
					}
					if(spos >= from + length){
						continue;
					}
					ForwardIterator it = output;
					if(spos > from){
						advance(it, spos - from);
					}
					for(size_type j = 0; j < m_param_k; ++j){
						if(spos + j >= from && spos + j < from + length){
							*it = mask(rshift(w, m_param_q - j - 1), 1);
							++it;
							--remain;
						}
					}
				}

				if(remain == 0){
					break;
				}
			}
		}

		return retval;
	}
}

#endif
