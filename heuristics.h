#pragma once
#include <map>
#include <string>

#include "board.h"

template <int N>
using Heuristic = int (*)(const typename Board<N>::State&);

template <int N> int zero(const typename Board<N>::State& s);       // h = 0 -> blind baseline
template <int N> int misplaced(const typename Board<N>::State& s);
template <int N> int manhattan(const typename Board<N>::State& s);

// Name -> heuristic for an NxN board.
// To plug a new one in: write it as template<int N>, add ONE line to the registry.
template <int N>
const std::map<std::string, Heuristic<N>>& heuristics();