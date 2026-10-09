#include "search.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <queue>
#include <tuple>
#include <unordered_map>

using Clock = std::chrono::steady_clock;

static double elapsed_ms(Clock::time_point t0) {
    return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
}

// ---------------------------------------------------------------- A*

template <int N>
Result astar(const typename Board<N>::State& start, Heuristic<N> h, const Limits& lim) {
    using B = Board<N>;
    using State = typename B::State;
    auto t0 = Clock::now();
    Result res;

    // Store only g + the move that reached the state; parent = apply inverse move.
    struct Info {
        int g;
        Move via;
        bool root;
    };
    using Node = std::tuple<int, int, State>;  // (f, -g, state): ties -> deeper first
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
    std::unordered_map<State, Info, StateHash> seen;
    seen.reserve(1 << 16);

    open.emplace(h(start), 0, start);
    seen[start] = {0, Move::Up, true};
    res.generated = 1;
    const State& goal = B::goal();

    while (!open.empty()) {
        auto [f, neg_g, s] = open.top();
        open.pop();
        int g = -neg_g;
        if (g > seen[s].g) continue;  // stale entry

        if (s == goal) {
            res.found = true;
            for (State cur = s;;) {
                const Info& in = seen[cur];
                if (in.root) break;
                res.path.push_back(in.via);
                cur = *B::apply_move(cur, inverse(in.via));
            }
            std::reverse(res.path.begin(), res.path.end());
            break;
        }
        if (lim.astar_max_expanded && res.expanded >= lim.astar_max_expanded) {
            res.aborted = true;
            break;
        }

        ++res.expanded;
        for (const auto& [m, t] : B::neighbours(s)) {
            int ng = g + 1;
            auto it = seen.find(t);
            if (it != seen.end() && it->second.g <= ng) continue;
            seen[t] = {ng, m, false};
            open.emplace(ng + h(t), -ng, t);
            ++res.generated;
        }
    }

    res.ms = elapsed_ms(t0);
    return res;
}

// ---------------------------------------------------------------- IDA*

namespace {
constexpr int FOUND = -1;
constexpr int INF = std::numeric_limits<int>::max();
constexpr int DR[] = {-1, 1, 0, 0};
constexpr int DC[] = {0, 0, -1, 1};

// Depth-first with an f-bound. Mutates one board in place: O(depth) memory.
template <int N>
struct Ida {
    using State = typename Board<N>::State;
    State s;
    int blank;
    Heuristic<N> h;
    const State& goal;
    size_t cap;
    Result& res;
    std::vector<Move> path;

    int dfs(int g, int bound, int prev) {
        int hv = h(s);
        int f = g + hv;
        if (f > bound) return f;
        if (hv == 0 && s == goal) return FOUND;  // admissible h: h > 0 => not goal
        if (cap && res.expanded >= cap) {
            res.aborted = true;
            return FOUND;  // unwind; caller checks res.aborted
        }
        ++res.expanded;

        int next = INF;
        int r = blank / N, c = blank % N;
        for (int k = 0; k < 4; ++k) {
            if (prev >= 0 && k == (prev ^ 1)) continue;  // never undo the last move
            int nr = r + DR[k], nc = c + DC[k];
            if (nr < 0 || nr >= N || nc < 0 || nc >= N) continue;

            int old = blank, nb = nr * N + nc;
            std::swap(s[old], s[nb]);
            blank = nb;
            path.push_back(static_cast<Move>(k));
            ++res.generated;

            int t = dfs(g + 1, bound, k);
            if (t == FOUND) return FOUND;
            next = std::min(next, t);

            path.pop_back();
            blank = old;
            std::swap(s[old], s[nb]);
        }
        return next;
    }
};
}  // namespace

template <int N>
Result idastar(const typename Board<N>::State& start, Heuristic<N> h, const Limits& lim) {
    using B = Board<N>;
    auto t0 = Clock::now();
    Result res;
    res.generated = 1;

    Ida<N> ida{start, B::blank_index(start), h, B::goal(), lim.ida_max_expanded, res, {}};
    ida.path.reserve(256);

    for (int bound = h(start);;) {
        ++res.iterations;
        int t = ida.dfs(0, bound, -1);
        if (t == FOUND) {
            if (!res.aborted) {
                res.found = true;
                res.path = ida.path;
            }
            break;
        }
        if (t == INF) break;  // exhausted: unsolvable
        bound = t;
    }

    res.ms = elapsed_ms(t0);
    return res;
}

// ---------------------------------------------------------------- registry

template <int N>
const std::map<std::string, SearchFn<N>>& searches() {
    static const std::map<std::string, SearchFn<N>> registry = {
        {"astar", astar<N>},
        {"idastar", idastar<N>},
    };
    return registry;
}

#define INSTANTIATE(N)                                                                  \
    template Result astar<N>(const Board<N>::State&, Heuristic<N>, const Limits&);      \
    template Result idastar<N>(const Board<N>::State&, Heuristic<N>, const Limits&);    \
    template const std::map<std::string, SearchFn<N>>& searches<N>();

INSTANTIATE(3)
INSTANTIATE(4)
INSTANTIATE(5)