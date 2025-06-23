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

using namespace std;
#define REP(i, n) for (ll i = 0; i < ll(n); i++)
#define FOR(i, a, b) for (ll i = a; i <= ll(b); i++)
#define ALL(x) x.begin(), x.end()

typedef long long ll;
typedef unsigned long long ull;

template <typename T> ostream &operator<<(ostream &os, const vector<T> &v);
ostream &operator<<(ostream &os, const vector<bool> &v);
template <typename T, typename U>
ostream &operator<<(ostream &os, const map<T, U> &v);
template <typename T, typename U>
ostream &operator<<(ostream &os, const unordered_map<T, U> &v);
template <typename T, typename U>
ostream &operator<<(ostream &os, const multimap<T, U> &v);
template <typename T> ostream &operator<<(ostream &os, const set<T> &v);
template <typename T>
ostream &operator<<(ostream &os, const unordered_set<T> &v);
template <typename T> ostream &operator<<(ostream &os, const multiset<T> &v);
template <typename T, typename U>
ostream &operator<<(ostream &os, const pair<T, U> &v);

template <typename T> ostream &operator<<(ostream &os, const vector<T> &v) {
  os << "[ ";
  for (auto const &x : v) {
    os << x;
    if (&x != &v.back()) {
      os << " , ";
    }
  }
  os << " ]";
  return os;
}

ostream &operator<<(ostream &os, const vector<bool> &v) {
  os << "[ ";
  for (size_t i = 0; i < v.size(); i++) {
    os << v[i];
    if (i != v.size() - 1) {
      os << " : ";
    }
  }
  os << " ]";
  return os;
}

// 汎用的なコンテナ出力ヘルパー
template <typename Container>
void print_container(ostream &os, const Container &v, const string &open = "{",
                     const string &close = " }") {
  os << open;
  for (auto const &x : v) {
    os << " " << x;
  }
  os << close;
}

template <typename T, typename U>
ostream &operator<<(ostream &os, const map<T, U> &v) {
  print_container(os, v, "{", "}");
  return os;
}
template <typename T, typename U>
ostream &operator<<(ostream &os, const unordered_map<T, U> &v) {
  print_container(os, v, "{", "}");
  return os;
}
template <typename T, typename U>
ostream &operator<<(ostream &os, const multimap<T, U> &v) {
  print_container(os, v, "{", "}");
  return os;
}
template <typename T> ostream &operator<<(ostream &os, const set<T> &v) {
  print_container(os, v);
  return os;
}
template <typename T>
ostream &operator<<(ostream &os, const unordered_set<T> &v) {
  print_container(os, v);
  return os;
}
template <typename T> ostream &operator<<(ostream &os, const multiset<T> &v) {
  print_container(os, v);
  return os;
}

template <typename T, typename U>
ostream &operator<<(ostream &os, const pair<T, U> &v) {
  os << "[" << v.first << ", " << v.second << " ]";
  return os;
}

void to_strstream(stringstream & /*ss*/) {}
template <class Head, class... Remain>
void to_strstream(stringstream &ss, Head &&head, Remain &&...remain) {
  ss << head << " ";
  to_strstream(ss, std::forward<Remain>(remain)...);
}

#ifdef ATCODERDEBUG
#define DEBUG_LOG(...)                                                         \
  if (true) {                                                                  \
    stringstream ss;                                                           \
    to_strstream(ss, __VA_ARGS__);                                             \
    cerr << "# " << (ss.str()) << endl;                                        \
  }
#else
#define DEBUG_LOG(...)
#endif

#define INFO_LOG(...)                                                          \
  if (true) {                                                                  \
    stringstream ss;                                                           \
    to_strstream(ss, __VA_ARGS__);                                             \
    cerr << "# " << (ss.str()) << endl;                                        \
  }

#ifdef ATCODERDEBUG
#define ASSERT_DEBUG(condition, ...)                                           \
  if (!(condition)) {                                                          \
    stringstream ss;                                                           \
    ss << "\033[1;31m";                                                        \
    to_strstream(ss, __VA_ARGS__);                                             \
    ss << "[" << __FILE__ << ":" << __LINE__ << "] \033[0m";                   \
    throw runtime_error(ss.str());                                             \
  }
#else
#define ASSERT_DEBUG(condition...)
#endif

template <typename T, typename U, typename Comp = less<>>
bool chmax(T &xmax, const U &x, Comp comp = {}) {
  if (comp(xmax, x)) {
    xmax = x;
    return true;
  }
  return false;
}

template <typename T, typename U, typename Comp = less<>>
bool chmin(T &xmin, const U &x, Comp comp = {}) {
  if (comp(x, xmin)) {
    xmin = x;
    return true;
  }
  return false;
}

const int IINF = 1012345678;
namespace myrandom {
uint64_t seed = 1234567891234567891;
uint64_t xorshift64() {
  seed ^= seed << 13;
  seed ^= seed >> 7;
  seed ^= seed << 17;
  return seed;
}
// l 以上 r 未満のランダムな整数を返す関数
int64_t next_int(int64_t l, int64_t r) {
  int64_t z = xorshift64() % (r - l);
  return l + z;
}
}; // namespace myrandom

inline double randf() {
  return (double)(myrandom::next_int(0, IINF)) / (double)IINF;
}

// generate v randomly that i1 <= v < i2
inline int randi_range(int i1, int i2) { return myrandom::next_int(i1, i2); }

struct TimeKeeper {
  struct RecordItem {
    string key;
    int count;
    double total;      // [s]
    double start_time; // [s]

    RecordItem(const string &_key = "default")
        : key(_key), count(0), total(0) {}
  };
  using time_point = chrono::steady_clock::time_point;

  time_point start_time;
  time_point end_time;
  unsigned int time_threshold_ms;
#ifdef ATCODERDEBUG
  bool enable_record = true;
#else
  bool enable_record = false;
#endif
  TimeKeeper(unsigned int _time_threshold_ms)
      : time_threshold_ms(_time_threshold_ms) {
    start_time = get_now();
    end_time = start_time + chrono::milliseconds(time_threshold_ms);
  }
  bool is_time_over() const { return get_now() >= end_time; }
  int get_elapsed_ms() const {
    auto delta = (get_now() - start_time);
    return chrono::duration_cast<chrono::milliseconds>(delta).count();
  }

  ll get_elapsed_ns() const {
    auto delta = (get_now() - start_time);
    return chrono::duration_cast<chrono::nanoseconds>(delta).count();
  }

  double get_elapsed_ratio() const {
    return (double)get_elapsed_ns() / 1e6 / (double)time_threshold_ms;
  }

  void start_record(const string &key) {
    if (!enable_record)
      return;
    if (record.count(key) == 0) {
      record[key] = RecordItem(key);
    }
    record[key].start_time = get_elapsed_ns();
  }

  void end_record(const string &key) {
    if (!enable_record)
      return;
    auto elapsed = get_elapsed_ns() - record[key].start_time;
    record[key].total += elapsed / 1e9;
    record[key].count++;
  }

  vector<RecordItem> get_record() {
    vector<RecordItem> res;
    for (auto &p : record) {
      res.push_back(p.second);
    }
    // sort by total
    sort(res.begin(), res.end(), [](const RecordItem &a, const RecordItem &b) {
      return a.total > b.total;
    });
    return res;
  }

private:
  unordered_map<string, RecordItem> record;

  time_point get_now() const { return chrono::steady_clock::now(); }
};

std::random_device seed_gen;
std::default_random_engine engine;
std::normal_distribution<> dist;

template <typename T> bool is_within(T v, T v1, T v2) {
  if (v1 > v2)
    swap(v1, v2);
  return v1 <= v && v <= v2;
}

static constexpr double EPSILON = 1e-8;
template <typename T> struct Mat2_2;

template <typename T> struct Vec2 {
  T x, y;
  Vec2(T x, T y) : x(x), y(y) {}
  Vec2() : x(0), y(0) {}
  Vec2 operator+(const Vec2 &v) const { return Vec2(x + v.x, y + v.y); }
  Vec2 operator+=(const Vec2 &v) {
    x += v.x;
    y += v.y;
    return *this;
  }
  Vec2 operator-(const Vec2 &v) const { return Vec2(x - v.x, y - v.y); }
  Vec2 operator-=(const Vec2 &v) {
    x -= v.x;
    y -= v.y;
    return *this;
  }
  Vec2 operator-() const { return Vec2(-x, -y); }
  Vec2 operator*(const T &s) const { return Vec2(x * s, y * s); }
  Vec2 operator*=(const T &s) {
    x *= s;
    y *= s;
    return *this;
  }
  Vec2 operator/(const T &s) const { return Vec2(x / s, y / s); }
  Vec2 operator/=(const T &s) {
    x /= s;
    y /= s;
    return *this;
  }
  bool operator==(const Vec2 &v) const { return (x == v.x && y == v.y); }
  bool operator!=(const Vec2 &v) const { return (x != v.x || y != v.y); }
  bool operator<(const Vec2 &v) const {
    if (x != v.x)
      return x < v.x;
    return y < v.y;
  }
  bool operator>(const Vec2 &v) const { return v < *this; }
  bool operator<=(const Vec2 &v) const { return !(v < *this); }
  bool operator>=(const Vec2 &v) const { return !(*this < v); }
  T dot(const Vec2 &v) const { return x * v.x + y * v.y; }
  Mat2_2<T> outer_product(const Vec2 &v) const {
    return Mat2_2<T>{Vec2<T>{x * v.x, y * v.x}, Vec2<T>{x * v.y, y * v.y}};
  }
  T norm() const { return std::sqrt(x * x + y * y); }
  void normalize() {
    auto length = norm();
    *this /= length;
  }
};
template <typename T> ostream &operator<<(ostream &os, const Vec2<T> &v) {
  os << "Vec2{" << v.x << ", " << v.y << "}";
  return os;
}

using Unit = double;
using Vec = Vec2<Unit>;
using Pos = Vec2<int>;
const Unit UINF = 1e8;
using Mat = Mat2_2<Unit>;

int man_dist(const Pos &p1, const Pos &p2) {
  return abs(p1.x - p2.x) + abs(p1.y - p2.y);
}
// int pos2int(const Pos &p) { return p.first * N + p.second; }
// Pos int2pos(int n) { return {n / N, n % N}; }

auto time_keeper = TimeKeeper(1920);

struct Solution {
  int score = 0;
};
struct Solver {

  double optimization_start_temp = 100.0;
  double optimization_end_temp = 0.01;
  Solver() {}

  void solve(Solution &sol) {}

  void output(const Solution &sol) const {}

  int calc_score(Solution &sol) {}
};

int main() {

  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(0);

  /**
   * Input
   */

  /**
   * Solve
   */
  Solver solver;
  Solution sol;

  solver.solve(sol);

  cerr << "Score = " << sol.score << endl;
#ifdef ATCODERDEBUG
#endif
}

/**
pahcer run --setting-file pahcer_config_single.toml

 */