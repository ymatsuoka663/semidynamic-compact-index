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

#include "semidynamic_compact_index.h"
#include <fstream>

namespace sdci{
	semidynamic_compact_index::semidynamic_compact_index()
	: m_sigma(), m_param_q(), m_param_k()
	{
		initialize(0, 0, 0);
	}

	semidynamic_compact_index::semidynamic_compact_index(
		size_type sigma_, size_type param_q_, size_type param_k_
	)
	: m_sigma(), m_param_q(), m_param_k()
	{
		initialize(sigma_, param_q_, param_k_);
	}

#if __cplusplus >= 201103L
	semidynamic_compact_index::semidynamic_compact_index(semidynamic_compact_index&& from)
	: m_sigma(), m_param_q(), m_param_k()
	{
		initialize(0, 0, 0);
		this->swap(from);
	}

	semidynamic_compact_index& semidynamic_compact_index::operator=
	(semidynamic_compact_index&& from){
		this->swap(from);
		return *this;
	}
#endif

	void semidynamic_compact_index::initialize
	(
		size_type sigma_, size_type param_q_, size_type param_k_
	)
	try{
		if(sigma_ + 1 == 0){
			sigma_ = m_sigma;
		}
		if(param_q_ + 1 == 0){
			param_q_ = m_param_q;
		}
		if(param_k_ + 1 == 0){
			param_k_ = m_param_k;
		}

		m_textlen = 0;
		m_last_qgram = 0;
		m_first_appearance = false;
		m_next_sampling_pos = param_q_;

		if(sigma_ == m_sigma && param_q_ == m_param_q && param_k_ == m_param_k){
			return;
		}

		m_sigma = 0;
		m_param_q = 0;
		m_param_k = 0;

		if(sigma_ == 0 || param_q_ == 0 || param_k_ == 0){
			return;
		}
		if(param_q_ < param_k_){
			throw std::invalid_argument("semidynamic_compact_index::initialize");
		}

		m_pow_sigma.resize(param_q_ + 1);
		m_pow_sigma[0] = 1;
		for(size_type i = 0; i < param_q_; ++i){
			m_pow_sigma[i + 1] = m_pow_sigma[i] * sigma_;
			if(m_pow_sigma[i + 1] < m_pow_sigma[i]){
				throw std::overflow_error("semidynamic_compact_index::initialize");
			}
		}

		const size_type kinds_of_qgrams = m_pow_sigma.back();
		if(kinds_of_qgrams > size_type(-1) / 8){
			throw std::overflow_error("semidynamic_compact_index::initialize");
		}

		size_type edge_width = ::sdci::detail::ceillg64(sigma_ + 1);
		m_list_sampled.initialize(kinds_of_qgrams);
		m_efirst.change_params(edge_width, kinds_of_qgrams);
		m_enext.change_params(edge_width, kinds_of_qgrams);
		m_encQ.initialize(kinds_of_qgrams);

		m_sigma = sigma_;
		m_param_q = param_q_;
		m_param_k = param_k_;
	}
	catch(...){
		m_pow_sigma.clear();
		m_list_sampled.clear();
		m_efirst.clear();
		m_enext.clear();
		m_encQ.initialize(0);
		throw;
	}

	void semidynamic_compact_index::reserve(size_type reserve_size_){
		if(m_param_k != 0){
			m_list_sampled.reserve((reserve_size_ + m_param_k - 1) / m_param_k);
		}
	}

	void semidynamic_compact_index::swap(semidynamic_compact_index &other){
		std::swap(m_sigma, other.m_sigma);
		std::swap(m_param_q, other.m_param_q);
		std::swap(m_param_k, other.m_param_k);
		std::swap(m_textlen, other.m_textlen);
		std::swap(m_last_qgram, other.m_last_qgram);
		std::swap(m_next_sampling_pos, other.m_next_sampling_pos);
		std::swap(m_first_appearance, other.m_first_appearance);
		m_list_sampled.swap(other.m_list_sampled);
		m_efirst.swap(other.m_efirst);
		m_enext.swap(other.m_enext);
		m_encQ.swap(other.m_encQ);
	}

	void semidynamic_compact_index::clear(){
		if(m_textlen >= m_param_q){
			if(m_textlen > m_param_q){
				m_efirst.fill0();
//				enext.fill0();	// unnecessary?
			}
			m_list_sampled.clear();
			m_encQ.clear();
		}

		m_textlen = 0;
		m_last_qgram = 0;
		m_next_sampling_pos = m_param_q;
		m_first_appearance = false;
	}

	void semidynamic_compact_index::invalidarg(encode_type value) const{
		std::ostringstream errmsg;
		errmsg << "Invalid argument: the alphabet size is " << m_sigma
			   << ", but the input contains the value \"" << value << "\".";
		throw std::invalid_argument(errmsg.str());
	}

	void semidynamic_compact_index::ptnlenerr() const{
		std::ostringstream errmsg;
		errmsg << "Length error: the length of pattern must not exceed "
			   << max_pattern_size() << ".";
		throw std::length_error(errmsg.str());
	}

	void semidynamic_compact_index::save_stream(std::ostream &stream) const{
		unsigned size_type_size = sizeof(size_type);
		::sdci::detail::write_data(stream, &size_type_size);
		::sdci::detail::write_data(stream, &m_sigma);
		::sdci::detail::write_data(stream, &m_param_q);
		::sdci::detail::write_data(stream, &m_param_k);
		::sdci::detail::write_data(stream, &m_textlen);
		::sdci::detail::write_data(stream, &m_last_qgram);
		::sdci::detail::write_data(stream, &m_next_sampling_pos);
		const char first_appearance = m_first_appearance;
		::sdci::detail::write_data(stream, &first_appearance);
		::sdci::detail::write_vector(stream, m_pow_sigma);
		
		m_list_sampled.save_stream(stream);
		m_efirst.save_stream(stream);
		m_enext.save_stream(stream);
		m_encQ.save_stream(stream);
	}

	void semidynamic_compact_index::save_file(const char *filename) const{
		std::ofstream stream(filename, std::ios_base::binary);
		if(!stream.good()){
			::sdci::detail::ioerr();
		}
		save_stream(stream);
	}

	void semidynamic_compact_index::load_stream(std::istream &stream) try{
		unsigned size_type_size = 0;
		::sdci::detail::read_data(stream, &size_type_size);
		if(size_type_size != sizeof(size_type)){
			::sdci::detail::formaterr();
		}
		
		::sdci::detail::read_data(stream, &m_sigma);
		::sdci::detail::read_data(stream, &m_param_q);
		::sdci::detail::read_data(stream, &m_param_k);
		::sdci::detail::read_data(stream, &m_textlen);
		::sdci::detail::read_data(stream, &m_last_qgram);
		::sdci::detail::read_data(stream, &m_next_sampling_pos);
		
		char first_appearance = 0;
		::sdci::detail::read_data(stream, &first_appearance);
		m_first_appearance = first_appearance;
		::sdci::detail::read_vector(stream, m_pow_sigma);
		
		m_list_sampled.load_stream(stream);
		m_efirst.load_stream(stream);
		m_enext.load_stream(stream);
		m_encQ.load_stream(stream);
	}
	catch(...){
		initialize(0, 0, 0);
		throw;
	}

	void semidynamic_compact_index::load_file(const char *filename){
		std::ifstream stream(filename, std::ios_base::binary);
		if(!stream.good()){
			::sdci::detail::ioerr();
		}
		load_stream(stream);
	}
}



