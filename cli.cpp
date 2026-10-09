#include "cli.h"

#include <cctype>
#include <cstdio>
#include <iostream>
#include <sstream>

static bool is_flag(const std::string& a) {
    return a.size() > 1 && a[0] == '-' && !std::isdigit(static_cast<unsigned char>(a[1]));
}

bool parse_args(int argc, char** argv, Options& opt, std::string& err) {
    auto need = [&](int& i, const std::string& a) -> const char* {
        if (++i >= argc) {
            err = "missing value for " + a;
            return nullptr;
        }
        return argv[i];
    };
    try {
        for (int i = 1; i < argc; ++i) {
            std::string a = argv[i];
            const char* v = nullptr;
            if (a == "--help" || a == "-h") {
                opt.help = true;
            } else if (a == "-n" || a == "--size") {
                if (!(v = need(i, a))) return false;
                opt.size = std::stoi(v);
                if (opt.size < 3 || opt.size > 5) { err = "size must be 3, 4 or 5"; return false; }
            } else if (a == "-H" || a == "--heuristic") {
                if (!(v = need(i, a))) return false;
                opt.heuristic = v;
            } else if (a == "-S" || a == "--search") {
                if (!(v = need(i, a))) return false;
                opt.search = v;
            } else if (a == "-r" || a == "--random") {
                if (!(v = need(i, a))) return false;
                opt.random_moves = std::stoi(v);
            } else if (a == "--seed") {
                if (!(v = need(i, a))) return false;
                opt.seed = std::stoull(v);
                opt.has_seed = true;
            } else if (a == "-m" || a == "--max-nodes") {
                if (!(v = need(i, a))) return false;
                opt.max_nodes = std::stoll(v);
            } else if (a == "-s" || a == "--steps") {
                opt.show_steps = true;
            } else if (a == "-l" || a == "--list") {
                opt.list = true;
            } else if (a == "-c" || a == "--compare") {
                opt.compare = true;
            } else if (is_flag(a)) {
                err = "unknown option: " + a;
                return false;
            } else {
                // positional board: one quoted string or many bare numbers
                std::string board = a;
                while (i + 1 < argc && !is_flag(argv[i + 1])) board += " " + std::string(argv[++i]);
                opt.board = board;
            }
        }
    } catch (...) {
        err = "bad numeric value";
        return false;
    }
    return true;
}

static std::string trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t\r\n"), e = s.find_last_not_of(" \t\r\n");
    return b == std::string::npos ? "" : s.substr(b, e - b + 1);
}

bool is_quit(const std::string& s) {
    std::string t = trim(s);
    return t == "q" || t == "Q" || t == "quit" || t == "exit";
}

int prompt_size(std::string& board_out, int last) {
    std::cerr << "Board size [3/4/5]";
    if (last) std::cerr << " (enter = " << last << ")";
    std::cerr << ", paste a board, or q to quit: ";
    for (std::string line; std::getline(std::cin, line);) {
        line = trim(line);
        if (is_quit(line)) return -1;
        if (line.empty()) {
            if (last) return last;
            continue;
        }
        if (line == "3" || line == "3x3") return 3;
        if (line == "4" || line == "4x4") return 4;
        if (line == "5" || line == "5x5") return 5;
        if (int n = infer_size(line)) {
            board_out = line;
            return n;
        }
        std::cerr << "Pick 3, 4 or 5: ";
    }
    return 0;
}

static size_t count_numbers(const std::string& s) {
    std::string cleaned;
    for (char c : s) cleaned += (c == ',' ? ' ' : c);
    std::istringstream ss(cleaned);
    size_t n = 0;
    for (std::string t; ss >> t;) n += (t.size() == 9 ? 9 : 1);  // compact 3x3 form
    return n;
}

std::string prompt_board(int n) {
    std::cerr << "Enter " << n << "x" << n << " board (" << n * n
              << " numbers, 0 or _ = blank), r [K] for a random K-move scramble, q to quit: ";
    std::string all;
    for (std::string line; std::getline(std::cin, line);) {
        line = trim(line);
        if (line.empty()) continue;
        if (is_quit(line)) return "q";
        if (all.empty() && (line[0] == 'r' || line[0] == 'R')) return line;
        all += line + " ";
        if (count_numbers(all) >= static_cast<size_t>(n * n)) break;
    }
    return all;
}

void print_usage(const char* prog) {
    std::cout
        << "Usage: " << prog << " [options] [board]\n\n"
        << "  board                 9/16/25 numbers, 0 or _ = blank (size is inferred)\n"
        << "                        3x3 also accepts compact form: 123405678\n"
        << "  -n, --size N          board size: 3, 4 or 5 (asked interactively if unknown)\n"
        << "  -S, --search NAME     search algorithm (default: idastar)\n"
        << "  -H, --heuristic NAME  heuristic (default: manhattan)\n"
        << "  -r, --random K        scramble K random moves from the goal\n"
        << "      --seed S          seed for --random (reproducible boards)\n"
        << "  -m, --max-nodes K     cap expanded nodes for every search (0 = unlimited)\n"
        << "                        defaults: A* 5M (RAM guard), IDA* 500M\n"
        << "  -s, --steps           print every board along the solution\n"
        << "  -c, --compare         run every search x heuristic and compare\n"
        << "  -l, --list            list registered searches and heuristics\n"
        << "  -h, --help            this message\n";
}

void print_list(const std::vector<std::string>& searches, const std::vector<std::string>& heuristics) {
    std::cout << "searches:\n";
    for (const auto& s : searches) std::cout << "  " << s << '\n';
    std::cout << "heuristics:\n";
    for (const auto& h : heuristics) std::cout << "  " << h << '\n';
}

static std::string fmt_time(double ms) {
    char buf[32];
    if (ms < 1.0)
        std::snprintf(buf, sizeof buf, "%.1f us", ms * 1000.0);
    else if (ms < 1000.0)
        std::snprintf(buf, sizeof buf, "%.2f ms", ms);
    else
        std::snprintf(buf, sizeof buf, "%.2f s", ms / 1000.0);
    return buf;
}

void print_result(const std::string& start_board, const Result& r, const std::string& sname,
                  const std::string& hname) {
    std::cout << "Start:\n" << start_board << '\n'
              << "Search     : " << sname << '\n'
              << "Heuristic  : " << hname << '\n';
    if (r.found) {
        std::cout << "Moves      : " << r.path.size() << '\n' << "Path       : ";
        for (Move m : r.path) std::cout << move_char(m);
        std::cout << '\n';
    } else if (r.aborted) {
        std::cout << "Result     : gave up, node cap hit (raise with -m, 0 = unlimited)\n";
    } else {
        std::cout << "Result     : no solution\n";
    }
    std::cout << "Expanded   : " << r.expanded << '\n' << "Generated  : " << r.generated << '\n';
    if (r.iterations) std::cout << "Iterations : " << r.iterations << '\n';
    std::cout << "Time       : " << fmt_time(r.ms) << '\n';
}

void print_steps(const std::vector<std::pair<Move, std::string>>& steps) {
    for (size_t i = 0; i < steps.size(); ++i)
        std::cout << "\n#" << i + 1 << " (" << move_char(steps[i].first) << ")\n" << steps[i].second;
}

void print_compare_header() {
    std::printf("%-8s %-10s %6s %12s %12s %10s\n", "search", "heuristic", "moves", "expanded",
                "generated", "time");
}

void print_compare_row(const std::string& sname, const std::string& hname, const Result& r) {
    std::string moves = r.found ? std::to_string(r.path.size()) : (r.aborted ? "cap" : "-");
    std::printf("%-8s %-10s %6s %12zu %12zu %10s\n", sname.c_str(), hname.c_str(), moves.c_str(),
                r.expanded, r.generated, fmt_time(r.ms).c_str());
    std::fflush(stdout);
}