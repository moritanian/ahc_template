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
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
/*
#include <atcoder/all>
template <int m, std::enable_if_t<(1 <= m)> * = nullptr>
ostream &operator<<(ostream &os, const atcoder::static_modint<m> &v) {
  os << v.val();
  return os;
}
*/

/*
cd $dir && g++ -std=c++17 -Wall -Wextra -O2 -DATCODERDEBUG
-I/home/moritanian/projects/atcoder/lib/ac-library $fileName && \
 echo 'compilation ok!'
&&  ./a.out
*/

using namespace std;
#define REP(i, n) for (ll i = 0; i < ll(n); i++)
#define FOR(i, a, b) for (ll i = a; i <= ll(b); i++)
#define ALL(x) x.begin(), x.end()
#define dame(a)                                                                \
  {                                                                            \
    cout << a << endl;                                                         \
    return 0;                                                                  \
  }
typedef long long ll;
typedef unsigned long long ull;

ll __per__(ll a, ll b) {
  if (a >= 0)
    return a % b;
  return (((-a - 1LL) / b) + 1LL) * b + a;
}

class dstream : private std::streambuf, public ostream {
public:
  dstream() : std::ostream(this) {}
  dstream &operator<<(__attribute__((unused))
                      ostream &(*endl)(std::ostream &out)) {
#ifdef ATCODERDEBUG
    cerr << endl;
#endif
    return *this;
  }
};

dstream dout;

template <typename T> dstream &operator<<(dstream &os, const T &v);
template <typename T> ostream &operator<<(ostream &os, const vector<T> &v);
ostream &operator<<(ostream &os, const vector<bool> &v);
// template <typename T>
// ostream &operator<<(ostream &os, const vector<vector<T>> &v);
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
template <class tTuple, std::size_t... indices>
void _print_tuple(ostream &os, tTuple const &iTuple,
                  std::index_sequence<indices...>);
template <typename T, typename U>
ostream &operator<<(ostream &os, const pair<T, U> &v);
template <class... ARGS>
ostream &operator<<(ostream &os, const tuple<ARGS...> &v);
template <typename T> ostream &operator<<(ostream &os, priority_queue<T> v);
template <typename T> ostream &operator<<(ostream &os, queue<T> v);

template <typename T> dstream &operator<<(dstream &os, const T &v) {
#ifdef ATCODERDEBUG
  cerr << "\033[1;31m" << v << "\033[0m";
#endif
  return os;
}

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
/*
template <typename T>
ostream &operator<<(ostream &os, const vector<vector<T>> &v) {
  os << "[";
  for (auto const &x : v) {
    os << " ";
    os << x;
  }
  os << "]";
  return os;
}
*/

template <typename T, typename U>
ostream &operator<<(ostream &os, const map<T, U> &v) {
  os << "{";
  for (auto const &x : v) {
    os << " ";
    os << x;
  }
  os << "}";
  return os;
}

template <typename T, typename U>
ostream &operator<<(ostream &os, const unordered_map<T, U> &v) {
  os << "{";
  for (auto const &x : v) {
    os << " ";
    os << x;
  }
  os << "}";
  return os;
}

template <typename T, typename U>
ostream &operator<<(ostream &os, const multimap<T, U> &v) {
  os << "{";
  for (auto const &x : v) {
    os << " ";
    os << x;
  }
  os << "}";
  return os;
}

template <typename T> ostream &operator<<(ostream &os, const set<T> &v) {
  os << "{";
  for (auto const &x : v) {
    os << " ";
    os << x;
  }
  os << " }";
  return os;
}

template <typename T>
ostream &operator<<(ostream &os, const unordered_set<T> &v) {
  os << "{";
  for (auto const &x : v) {
    os << " ";
    os << x;
  }
  os << " }";
  return os;
}

template <typename T> ostream &operator<<(ostream &os, const multiset<T> &v) {
  os << "{";
  for (auto const &x : v) {
    os << " ";
    os << x;
  }
  os << " }";
  return os;
}

template <typename T, typename U>
ostream &operator<<(ostream &os, const pair<T, U> &v) {
  os << "[" << v.first << ", " << v.second << " ]";
  return os;
}

template <class tTuple, std::size_t... indices>
void _print_tuple(ostream &os, tTuple const &iTuple,
                  std::index_sequence<indices...>) {
  using swallow = int[];
  os << "[ ";
  (void)swallow{(os << std::get<indices>(iTuple) << " , ", 0)...};
  os << " ]";
}

template <class... ARGS>
ostream &operator<<(ostream &os, const tuple<ARGS...> &v) {
  constexpr size_t N = tuple_size<tuple<ARGS...>>::value;
  _print_tuple(os, v, std::make_index_sequence<N>{});
  return os;
}

template <typename T> ostream &operator<<(ostream &os, priority_queue<T> v) {
  os << "[";
  while (!v.empty()) {
    os << v.top();
    v.pop();
    if (!v.empty()) {
      os << ", ";
    }
  }
  os << "]";
  return os;
}

template <typename T> ostream &operator<<(ostream &os, queue<T> v) {
  os << "[";
  while (!v.empty()) {
    os << v.front();
    v.pop();
    if (!v.empty()) {
      os << ", ";
    }
  }
  os << "]";
  return os;
}

void to_strstream(stringstream & /*ss*/) {}
template <class Head, class... Remain>
void to_strstream(stringstream &ss, Head &&head, Remain &&...remain) {
  ss << head << " ";
  to_strstream(ss, forward<Remain>(remain)...);
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

ll log10ll(ll n) {
  ll res = 0;
  while (n >= 10) {
    n /= 10LL;
    res++;
  }
  return res;
}

struct UF {
  vector<ll> data;
  UF(ll size) : data(size, -1) {}
  bool unite(ll x, ll y) {
    x = root(x);
    y = root(y);
    if (x != y) {
      if (-data[y] > -data[x])
        swap(x, y);
      data[x] += data[y];
      data[y] = x;
    }
    return x != y;
  }
  bool findSet(ll x, ll y) { return root(x) == root(y); }
  ll root(ll x) { return data[x] < 0 ? x : data[x] = root(data[x]); }
  ll size(ll x) { return -data[root(x)]; }
};

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

int div_ceil(int a, int b) { return (a + b - 1) / b; }

double sigmoid(double x) { return 1.0 / (1.0 + exp(-x)); }
double abs_radian(double radian) {
  while (radian < -M_PI)
    radian += 2 * M_PI;
  while (radian >= M_PI)
    radian -= 2 * M_PI;
  return abs(radian);
}

string get_env_string(const string &key, const string &default_value) {
  const char *value = getenv(key.c_str());
  if (value == nullptr) {
    return default_value;
  }
  return string(value);
}

double get_env_double(const string &key, double default_value) {
  const char *value = getenv(key.c_str());
  if (value == nullptr) {
    return default_value;
  }
  return atof(value);
}

int get_env_int(const string &key, int default_value) {
  const char *value = getenv(key.c_str());
  if (value == nullptr) {
    return default_value;
  }
  return atoi(value);
}

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

template <typename T> struct Mat2_2 {
  Vec2<T> a, b;
  Mat2_2(const Vec2<T> &a, const Vec2<T> &b) : a(a), b(b) {}
  Mat2_2() : a(), b() {}
  Mat2_2 operator+(const Mat2_2 &m) const { return Mat2_2(a + m.a, b + m.b); }
  Mat2_2 operator+=(const Mat2_2 &m) {
    a += m.a;
    b += m.b;
    return *this;
  }
  Mat2_2 operator-(const Mat2_2 &m) const { return Mat2_2(a - m.a, b - m.b); }
  Mat2_2 operator-=(const Mat2_2 &m) {
    a -= m.a;
    b -= m.b;
    return *this;
  }
  Mat2_2 operator-() const { return Mat2_2(-a, -b); }

  Mat2_2 operator*(const T &s) const { return Mat2_2(a * s, b * s); }
  Mat2_2 operator/(const T &s) const { return Mat2_2(a / s, b / s); }

  Vec2<T> dot(const Vec2<T> &v) const {
    return Vec2<T>{a.x * v.x + b.x * v.y, a.y * v.x + b.y * v.y};
  }
  Mat2_2 dot(const Mat2_2 &m) const { return Mat2_2<T>{dot(m.a), dot(m.b)}; }

  Mat2_2 inverse() const {
    T _det = det();
    if (_det == 0) {
      DEBUG_LOG("det == 0!!!");
      _det = EPSILON;
    }
    return Mat2_2<T>{Vec2<T>{b.y, -a.y}, Vec2<T>{-b.x, a.x}} / _det;
  }

  T trace() const { return a.x + b.y; }
  T det() const { return a.x * b.y - a.y * b.x; }
};

template <typename T> ostream &operator<<(ostream &os, const Mat2_2<T> &m) {
  os << "Mat2_2{" << m.a << "^T, " << m.b << "^T}";
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

namespace Gaussian {
// 標準正規分布の確率密度関数
Unit phi(Unit x) {
  return (1.0 / std::sqrt(2.0 * M_PI)) * std::exp(-0.5 * x * x);
}

// 標準正規分布の累積分布関数
Unit Phi(Unit x) { return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0))); }
} // namespace Gaussian

auto time_keeper = TimeKeeper(1920);

struct Solution {
  int score = 0;
};
struct Solver {

  double optimization_start_temp;
  double optimization_end_temp;
  Solver() {
    optimization_start_temp = get_env_double("OPTIMIZATION_START_TEMP", 100.0);
    optimization_end_temp = get_env_double("OPTIMIZATION_END_TEMP", 0.01);
  }

  void solve(Solution &sol) {}

  void optimize(Solution &solution, double over_ratio = 1.0) {
    INFO_LOG("# annealing", time_keeper.get_elapsed_ms(), "score",
             solution.score);

    const double start_temp = optimization_start_temp;
    const double end_temp = optimization_end_temp;

    const double check_time_elapsed_ratio = 0.002;
    int check_time_freq = 1;
    const int num_time_sample = 100;

    auto best_score = solution.score;

    auto best_solution = solution;

    auto init_time = time_keeper.get_elapsed_ratio();
    /***************************************************************/

    auto now_solution = solution;
    auto now_score = best_score;
    double crt_elapsed_ratio = time_keeper.get_elapsed_ratio();
    int update_cnt = 0;
    int improved_cnt = 0;

    for (int lp = 0;; lp++) {
      /*************************************************************/

      // Update check_time_freq so that the exec time of check_time_freq times
      // loop is check_time_elapsed_ratio
      if (lp == num_time_sample) {
        crt_elapsed_ratio = time_keeper.get_elapsed_ratio();
        auto elapsed_ratio = max(crt_elapsed_ratio - init_time, 0.00001);
        check_time_freq =
            num_time_sample * check_time_elapsed_ratio / elapsed_ratio;
        if (check_time_freq < 1) {
          check_time_freq = 1;
        }
        if (check_time_freq > 500) {
          check_time_freq = 500;
        }
        INFO_LOG("updated check_time_freq", check_time_freq, "elapsed_ratio",
                 elapsed_ratio, "crt_elapsed_ratio", crt_elapsed_ratio);
      }

      if (lp % check_time_freq == 0) {

        crt_elapsed_ratio = time_keeper.get_elapsed_ratio();

        if (crt_elapsed_ratio >= over_ratio) {
          INFO_LOG("#over", best_score, "lp", lp, "update", update_cnt,
                   "improved", improved_cnt, "elapsed",
                   time_keeper.get_elapsed_ms());
          DEBUG_LOG("!D!", update_cnt);
          solution = std::move(best_solution);
          break;
        }
      }
      // end of 3
      /*************************************************************/

      /*************************************************************/
      auto elapsed_ratio =
          (crt_elapsed_ratio - init_time) / ((over_ratio - init_time));
      double temp = start_temp + (end_temp - start_temp) * elapsed_ratio;
      double diff_threshold = temp * log(randf());
      // end of 4
      /************************************************************/

      /*************************************************************/
      auto next_score = calc_neighbor_ans(now_solution, now_score,
                                          diff_threshold, crt_elapsed_ratio);
      // end of 5
      /*************************************************************/

      /*************************************************************/
      auto diff = next_score - now_score;
      if (diff < diff_threshold) {
        continue;
      }
      // DEBUG_LOG("next_score", next_score, "now_score", now_score);

      now_score = next_score;
      update_cnt++;

      if (chmax(best_score, now_score)) {
        best_solution = now_solution;
        improved_cnt++;

        DEBUG_LOG("update by annealing!!", best_score, "lp", lp, "update",
                  update_cnt, "elapsed ratio", elapsed_ratio);
        // dout << "update !! " << high_score << high_answer_ << endl;
      }
    }
  }

  Unit calc_neighbor_ans(Solution &solution, int score, double diff_threshold,
                         double elapsed_ratio) {
    // TODO
    return -UINF;
  }
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

  // INFO_LOG("!D!", M);
  // INFO_LOG("!D!", L);

#ifdef ATCODERDEBUG
#endif
}

/**
pahcer run --setting-file pahcer_config_single.toml

 */