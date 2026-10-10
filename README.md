# N-Puzzle Solver

> ***Professor: Leyla Muradkhanli***    \
> ***Student: Huseyn Movla***         \
> ***Subject: Foundations of Artificial Intelligence***

Optimal solver for the 3×3, 4×4 and 5×5 sliding puzzles (8-, 15- and 24-puzzle) in C++17.
Search algorithms and heuristics are pluggable.

## Build

```sh
make
```

## Usage

**Interactive loop:** run with no board. Type `q` to quit.

```sh
./puzzle
```

At the board prompt, enter a board, or type `r 40` for a random 40-move scramble.
At the size prompt, pressing Enter reuses the last size.

**One-shot:** pass a board on the command line. The size is inferred from the tile count (9, 16 or 25).

```sh
./puzzle 123405678                      # 3x3, compact form
./puzzle "1 2 3 4 5 6 7 8 9 10 11 12 13 14 0 15"
./puzzle -n 4 -r 60 --seed 7            # random 4x4, reproducible
./puzzle -c 867254301                   # compare every search x heuristic
```

`0` or `_` marks the blank. The goal has the tiles in order with the blank **bottom-right**.

| Flag | Meaning |
|---|---|
| `-n 3\|4\|5` | board size |
| `-S astar\|idastar` | search algorithm (default `idastar`) |
| `-H zero\|misplaced\|manhattan` | heuristic (default `manhattan`) |
| `-r K`, `--seed S` | random K-move scramble |
| `-m K` | node cap, `0` = unlimited (defaults: A* 5M, IDA* 500M) |
| `-s` | print every step |
| `-c` | comparison table |
| `-l` | list searches and heuristics |

Output moves (`U D L R`) are the direction the **blank** moves.

## Layout

```
main.cpp        wiring: size dispatch, interactive loop
board.*         Board<N>: state, moves, solvability, parse/print, scramble
heuristics.*    heuristics + registry
search.*        A*, IDA* + registry
cli.*           args, prompts, output
```

## Adding a heuristic

1. Write `template <int N> int my_h(const typename Board<N>::State&);` in `heuristics.cpp`.
2. Add `{"myh", my_h<N>}` to the registry, plus one line in the `INSTANTIATE` macro.

It is then available via `-H myh`, `-l` and `-c`. Search algorithms plug in the same way in `search.cpp`.

## Notes

- Unsolvable boards are rejected up front by an inversion-parity check. That rules out half of all arrangements.
- A* is limited by memory and IDA* by time. Use IDA* for 4×4 and up.
- With Manhattan, fully random 5×5 boards are out of reach. That needs Linear Conflict or pattern databases.