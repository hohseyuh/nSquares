#include "board.h"

#include <cctype>
#include <iomanip>
#include <random>
#include <sstream>

static constexpr int DR[] = {-1, 1, 0, 0};
static constexpr int DC[] = {0, 0, -1, 1};

char move_char(Move m) {
    switch (m) {
        case Move::Up: return 'U';
        case Move::Down: return 'D';
        case Move::Left: return 'L';
        case Move::Right: return 'R';
    }
    return '?';
}

// "1 2 3 ...", "1,2,3,...", "_" = blank, or compact "123405678" (3x3 only).
static bool tokenize(const std::string& in, std::vector<int>& vals) {
    std::string cleaned;
    for (char c : in) cleaned += (c == ',' ? ' ' : (c == '_' ? '0' : c));

    std::vector<std::string> toks;
    std::istringstream ss(cleaned);
    for (std::string t; ss >> t;) toks.push_back(t);

    vals.clear();
    if (toks.size() == 1 && toks[0].size() == 9) {
        for (char c : toks[0]) {
            if (!std::isdigit(static_cast<unsigned char>(c))) return false;
            vals.push_back(c - '0');
        }
        return true;
    }
    for (const auto& t : toks) {
        if (t.size() > 3) return false;
        for (char c : t)
            if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        vals.push_back(std::stoi(t));
    }
    return true;
}

int infer_size(const std::string& text) {
    std::vector<int> v;
    if (!tokenize(text, v)) return 0;
    switch (v.size()) {
        case 9: return 3;
        case 16: return 4;
        case 25: return 5;
        default: return 0;
    }
}

template <int N>
const typename Board<N>::State& Board<N>::goal() {
    static const State g = [] {
        State s{};
        for (int i = 0; i < CELLS - 1; ++i) s[i] = static_cast<uint8_t>(i + 1);
        s[CELLS - 1] = 0;
        return s;
    }();
    return g;
}

template <int N>
int Board<N>::blank_index(const State& s) {
    for (int i = 0; i < CELLS; ++i)
        if (s[i] == 0) return i;
    return -1;
}

template <int N>
bool Board<N>::is_solvable(const State& s) {
    int inv = 0;
    for (int i = 0; i < CELLS; ++i)
        for (int j = i + 1; j < CELLS; ++j)
            if (s[i] && s[j] && s[i] > s[j]) ++inv;
    if (N % 2 == 1) return inv % 2 == 0;
    int row_from_bottom = N - blank_index(s) / N;  // 1-indexed
    return (inv + row_from_bottom) % 2 == 1;
}

template <int N>
std::optional<typename Board<N>::State> Board<N>::apply_move(const State& s, Move m) {
    int b = blank_index(s);
    int k = static_cast<int>(m);
    int nr = b / N + DR[k], nc = b % N + DC[k];
    if (nr < 0 || nr >= N || nc < 0 || nc >= N) return std::nullopt;
    State t = s;
    std::swap(t[b], t[nr * N + nc]);
    return t;
}

template <int N>
std::vector<std::pair<Move, typename Board<N>::State>> Board<N>::neighbours(const State& s) {
    std::vector<std::pair<Move, State>> out;
    out.reserve(4);
    for (Move m : ALL_MOVES)
        if (auto t = apply_move(s, m)) out.emplace_back(m, *t);
    return out;
}

template <int N>
typename Board<N>::State Board<N>::scramble(int moves, uint64_t seed) {
    std::mt19937_64 rng(seed);
    State s = goal();
    int prev = -1;
    for (int i = 0; i < moves; ++i) {
        std::vector<std::pair<Move, State>> opts;
        for (auto& [m, t] : neighbours(s))
            if (prev < 0 || static_cast<int>(m) != (prev ^ 1)) opts.emplace_back(m, t);
        auto& pick = opts[rng() % opts.size()];
        prev = static_cast<int>(pick.first);
        s = pick.second;
    }
    return s;
}

template <int N>
bool Board<N>::parse(const std::string& in, State& out) {
    std::vector<int> vals;
    if (!tokenize(in, vals) || vals.size() != static_cast<size_t>(CELLS)) return false;
    std::array<bool, CELLS> seen{};
    for (int i = 0; i < CELLS; ++i) {
        int v = vals[i];
        if (v < 0 || v >= CELLS || seen[v]) return false;
        seen[v] = true;
        out[i] = static_cast<uint8_t>(v);
    }
    return true;
}

template <int N>
std::string Board<N>::to_string(const State& s) {
    const int w = N > 3 ? 2 : 1;
    std::ostringstream os;
    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            int v = s[r * N + c];
            os << std::setw(w) << (v ? std::to_string(v) : "_") << (c + 1 < N ? " " : "");
        }
        os << '\n';
    }
    return os.str();
}

template struct Board<3>;
template struct Board<4>;
template struct Board<5>;