#include <algorithm>
#include <array>
#include <bits/types/timer_t.h>
#include <cassert>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// #include "lib/edge_beam.cpp"
#include "lib/my_lib.cpp"

using namespace std;

auto time_keeper = TimeKeeper(1920);

struct Input {

  void input() {}
};

struct Solution {
  int score;
};

struct Solver {

  Input input;
  Solver(Input _input) : input(_input) {}

  void solve(Solution &sol) {}

  // 出力
  void output(const Solution &sol) const {}

  int calc_score(const Solution &sol) { return 0; }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(0);

  /**
   * Input
   */
  Input input;
  input.input();

  /**
   * Solve
   */
  Solver solver(input);
  Solution sol;

  solver.solve(sol);

  /**
   * Output
   */
  solver.output(sol);

  cerr << "Score = " << sol.score << endl;
}
