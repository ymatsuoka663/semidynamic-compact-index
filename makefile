CXX = g++
CXXFLAGS = -O2 -Wall -std=c++11

all: sdci.a example

clean:
	rm -f *.o sdci.a

sdci.a: sampled_position_list.o integer_set.o packed_array.o \
 semidynamic_compact_index.o
	ar rc sdci.a sampled_position_list.o integer_set.o packed_array.o semidynamic_compact_index.o

sampled_position_list.o: sampled_position_list.cpp \
 sampled_position_list.h sdci_common.h packed_array.h
	$(CXX) $(CXXFLAGS) -c -o sampled_position_list.o sampled_position_list.cpp

integer_set.o: integer_set.cpp integer_set.h sdci_common.h
	$(CXX) $(CXXFLAGS) -c -o integer_set.o integer_set.cpp

packed_array.o: packed_array.cpp packed_array.h sdci_common.h
	$(CXX) $(CXXFLAGS) -c -o packed_array.o packed_array.cpp

semidynamic_compact_index.o: semidynamic_compact_index.cpp \
 semidynamic_compact_index.h sdci_common.h integer_set.h \
 sampled_position_list.h packed_array.h sdci_impl.h
	$(CXX) $(CXXFLAGS) -c -o semidynamic_compact_index.o semidynamic_compact_index.cpp

example: sdci.a example.cpp
	$(CXX) $(CXXFLAGS) -o example example.cpp sdci.a
