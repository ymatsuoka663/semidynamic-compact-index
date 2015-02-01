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

#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include "semidynamic_compact_index.h"

// convert 'a'->0, 'b'->1, 'c'->2 and 'd'->3.
void convert(const std::string &from, std::vector<std::size_t> &to){
	to.resize(from.size());
	for(std::size_t i = 0; i < from.size(); ++i){
		to[i] = from[i] - 'a';
	}
}

void output(const std::vector<std::size_t> &v){
	std::cout << '{';
	for(std::vector<std::size_t>::const_iterator it = v.begin(); it != v.end(); ++it){
		if(it != v.begin()){ std::cout << ','; }
		std::cout << *it;
	}
	std::cout << '}' << std::endl;
}

int main(){
	const std::size_t sigma = 4;
	const std::size_t q = 6;
	const std::size_t k = 3;
	// maximum pattern length is q-k+1=4.

	const std::string original_text = "abcadccbacbcabcadb";
	const std::string append_text = "caddacbd";
	const std::string pattern = "bca";
	std::vector<std::size_t> tmp1, tmp2, tmp3;
	std::vector<std::size_t> result1, result2;

	sdci::semidynamic_compact_index idx;
	idx.initialize(sigma, q, k);

	convert(original_text, tmp1);
	idx.assign(tmp1.begin(), tmp1.end());
	// text: abcadccbacbcabcadb

	convert(pattern, tmp2);
	idx.locate(tmp2.begin(), tmp2.end(), std::back_inserter(result1));
	output(result1);	// {1,10,13} in any order

	convert(append_text, tmp3);
	idx.append(tmp3.begin(), tmp3.end());
	// text: abcadccbacbcabcadbcaddacbd

	idx.locate(tmp2.begin(), tmp2.end(), std::back_inserter(result2));
	output(result2);	// {1,10,13,17} in any order
}

