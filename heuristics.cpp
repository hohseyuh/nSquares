#include "heuristics.h"

#include <cstdlib>

template <int N>
int zero(const typename Board<N>::State&) {
    return 0;
}

template <int N>
int misplaced(const typename Board<N>::State& s) {
    const auto& g = Board<N>::goal();
    int h = 0;
    for (int i = 0; i < Board<N>::CELLS; ++i)
        if (s[i] && s[i] != g[i]) ++h;
    return h;
}

template <int N>
int manhattan(const typename Board<N>::State& s) {
    int h = 0;
    for (int i = 0; i < Board<N>::CELLS; ++i) {
        int v = s[i];
        if (!v) continue;
        // goal: tile v sits at index v-1
        h += std::abs(i / N - (v - 1) / N) + std::abs(i % N - (v - 1) % N);
    }
    return h;
}

template <int N>
const std::map<std::string, Heuristic<N>>& heuristics() {
    static const std::map<std::string, Heuristic<N>> registry = {
        {"zero", zero<N>},
        {"misplaced", misplaced<N>},
        {"manhattan", manhattan<N>},
        // {"linear", linear_conflict<N>},   <- plug the next one here
    };
    return registry;
}

#define INSTANTIATE(N)                                                   \
    template int zero<N>(const Board<N>::State&);                        \
    template int misplaced<N>(const Board<N>::State&);                   \
    template int manhattan<N>(const Board<N>::State&);                   \
    template const std::map<std::string, Heuristic<N>>& heuristics<N>();

INSTANTIATE(3)
INSTANTIATE(4)
INSTANTIATE(5)