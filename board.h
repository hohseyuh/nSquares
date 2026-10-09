#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// Direction the BLANK moves. Order matters: k ^ 1 is the inverse move.
enum class Move : uint8_t { Up, Down, Left, Right };
constexpr Move ALL_MOVES[] = {Move::Up, Move::Down, Move::Left, Move::Right};
inline Move inverse(Move m) { return static_cast<Move>(static_cast<int>(m) ^ 1); }
char move_char(Move m);

// 9 / 16 / 25 tiles -> 3 / 4 / 5, anything else -> 0.
int infer_size(const std::string& text);

// Supported sizes are instantiated in the .cpp files (3, 4, 5).
// Adding 6x6 = one INSTANTIATE line per .cpp + one case in main.
template <int N>
struct Board {
    static_assert(N >= 2 && N <= 15, "tiles must fit in uint8_t");
    static constexpr int CELLS = N * N;
    using State = std::array<uint8_t, CELLS>;  // 0 = blank

    static const State& goal();
    static int blank_index(const State& s);
    static bool is_solvable(const State& s);
    static std::optional<State> apply_move(const State& s, Move m);
    static std::vector<std::pair<Move, State>> neighbours(const State& s);
    static State scramble(int moves, uint64_t seed);  // random walk from goal
    static bool parse(const std::string& in, State& out);
    static std::string to_string(const State& s);
};

// FNV-1a over the tiles: works for any size, no 64-bit packing limit.
struct StateHash {
    template <size_t K>
    size_t operator()(const std::array<uint8_t, K>& s) const noexcept {
        uint64_t h = 0xcbf29ce484222325ULL;
        for (uint8_t v : s) {
            h ^= v;
            h *= 0x100000001b3ULL;
        }
        return static_cast<size_t>(h);
    }
};