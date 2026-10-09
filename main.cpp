#include <iostream>
#include <random>
#include <sstream>

#include "board.h"
#include "cli.h"
#include "heuristics.h"
#include "search.h"

template <int N>
int run(Options& opt) {
    using B = Board<N>;

    if (opt.list) {
        std::vector<std::string> s, h;
        for (const auto& kv : searches<N>()) s.push_back(kv.first);
        for (const auto& kv : heuristics<N>()) h.push_back(kv.first);
        print_list(s, h);
        return 0;
    }

    typename B::State start;
    if (opt.random_moves >= 0) {
        uint64_t seed = opt.has_seed ? opt.seed : std::random_device{}();
        start = B::scramble(opt.random_moves, seed);
        std::cout << "Scrambled " << opt.random_moves << " random moves (seed " << seed << ")\n";
    } else if (!B::parse(opt.board, start)) {
        std::cerr << "error: invalid " << N << "x" << N << " board (need each of 0.." << B::CELLS - 1
                  << " exactly once)\n";
        return 1;
    }
    if (!B::is_solvable(start)) {
        std::cerr << "Unsolvable board (wrong parity). Not wasting cycles on it.\n";
        return 2;
    }

    Limits lim;
    if (opt.max_nodes >= 0) lim.astar_max_expanded = lim.ida_max_expanded = opt.max_nodes;

    // ---- compare mode: every search x every heuristic
    if (opt.compare) {
        std::cout << B::to_string(start) << '\n';
        print_compare_header();
        for (const auto& [sn, sf] : searches<N>())
            for (const auto& [hn, hf] : heuristics<N>()) print_compare_row(sn, hn, sf(start, hf, lim));
        return 0;
    }

    // ---- single run
    auto sit = searches<N>().find(opt.search);
    auto hit = heuristics<N>().find(opt.heuristic);
    if (sit == searches<N>().end() || hit == heuristics<N>().end()) {
        std::cerr << "error: unknown " << (sit == searches<N>().end() ? "search '" + opt.search
                                                                      : "heuristic '" + opt.heuristic)
                  << "'. Use -l to list options.\n";
        return 1;
    }

    Result r = sit->second(start, hit->second, lim);
    print_result(B::to_string(start), r, sit->first, hit->first);

    if (opt.show_steps && r.found) {
        std::vector<std::pair<Move, std::string>> steps;
        auto s = start;
        for (Move m : r.path) {
            s = *B::apply_move(s, m);
            steps.emplace_back(m, B::to_string(s));
        }
        print_steps(steps);
    }
    return r.found ? 0 : 3;
}

static int dispatch(int n, Options& opt) {
    switch (n) {
        case 3: return run<3>(opt);
        case 4: return run<4>(opt);
        case 5: return run<5>(opt);
        default:
            std::cerr << "error: size must be 3, 4 or 5\n";
            return 1;
    }
}

int main(int argc, char** argv) {
    Options opt;
    std::string err;
    if (!parse_args(argc, argv, opt, err)) {
        std::cerr << "error: " << err << "\n\n";
        print_usage(argv[0]);
        return 1;
    }
    if (opt.help) {
        print_usage(argv[0]);
        return 0;
    }

    if (opt.list) return run<3>(opt);  // registries are the same for every size

    // ---- one-shot: board or --random given on the command line
    if (!opt.board.empty() || opt.random_moves >= 0) {
        int n = opt.size;
        if (!n) n = opt.board.empty() ? 3 : infer_size(opt.board);
        if (!n) {
            std::cerr << "error: can't tell the size, a board needs 9, 16 or 25 tiles\n";
            return 1;
        }
        return dispatch(n, opt);
    }

    // ---- interactive loop until q (or EOF)
    int last = opt.size;
    for (;;) {
        Options o = opt;
        int n = prompt_size(o.board, last);
        if (n <= 0) break;
        last = n;

        if (o.board.empty()) {
            std::string in = prompt_board(n);
            if (in.empty() || is_quit(in)) break;
            if (in[0] == 'r' || in[0] == 'R') {
                std::istringstream ss(in.substr(1));
                int k = 30;
                ss >> k;
                o.random_moves = k;
            } else {
                o.board = in;
            }
        }
        dispatch(n, o);
        std::cout << "\n" << std::string(40, '-') << "\n\n";
    }
    std::cerr << "\nBye.\n";
    return 0;
}