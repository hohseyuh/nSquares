#pragma once
#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "board.h"
#include "heuristics.h"

// Node caps (0 = unlimited). A* is memory-bound, IDA* only time-bound.
struct Limits {
    size_t astar_max_expanded = 5'000'000;
    size_t ida_max_expanded = 500'000'000;
};

struct Result {
    bool found = false;
    bool aborted = false;  // hit the node cap
    std::vector<Move> path;
    size_t expanded = 0;
    size_t generated = 0;
    int iterations = 0;  // IDA* threshold rounds (0 for A*)
    double ms = 0.0;
};

template <int N>
using SearchFn = Result (*)(const typename Board<N>::State&, Heuristic<N>, const Limits&);

template <int N>
Result astar(const typename Board<N>::State& start, Heuristic<N> h, const Limits& lim);

template <int N>
Result idastar(const typename Board<N>::State& start, Heuristic<N> h, const Limits& lim);

// Name -> search algorithm, pluggable the same way as heuristics.
template <int N>
const std::map<std::string, SearchFn<N>>& searches();