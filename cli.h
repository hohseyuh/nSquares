#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "board.h"
#include "search.h"

// Size-independent: everything here deals in strings and Results.
struct Options {
    int size = 0;            // 0 = infer from board, else ask
    std::string board;       // raw text, parsed once the size is known
    int random_moves = -1;   // >= 0 -> scramble that many moves from goal
    uint64_t seed = 0;
    bool has_seed = false;
    std::string heuristic = "manhattan";
    std::string search = "idastar";
    long long max_nodes = -1;  // -1 = defaults, 0 = unlimited
    bool show_steps = false;
    bool list = false;
    bool compare = false;
    bool help = false;
};

bool parse_args(int argc, char** argv, Options& opt, std::string& err);

// Interactive. prompt_size also accepts a pasted board (stored in board_out).
// Returns 3/4/5, -1 on q, 0 on EOF. Empty input reuses `last` (if nonzero).
int prompt_size(std::string& board_out, int last);
// Returns board text, "r" / "r K" for a random scramble, "q" to quit. Empty on EOF.
std::string prompt_board(int n);
bool is_quit(const std::string& s);

void print_usage(const char* prog);
void print_list(const std::vector<std::string>& searches, const std::vector<std::string>& heuristics);
void print_result(const std::string& start_board, const Result& r, const std::string& sname,
                  const std::string& hname);
void print_steps(const std::vector<std::pair<Move, std::string>>& steps);
void print_compare_header();
void print_compare_row(const std::string& sname, const std::string& hname, const Result& r);