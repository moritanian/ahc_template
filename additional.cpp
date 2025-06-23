#include <algorithm>
#include <bits/types/timer_t.h>
#include <cmath>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

using namespace std;
#define REP(i, n) for (ll i = 0; i < ll(n); i++)
#define FOR(i, a, b) for (ll i = a; i <= ll(b); i++)
#define ALL(x) x.begin(), x.end()

typedef long long ll;
typedef unsigned long long ull;
using Unit = double;

// -----------------------------------------------------------------

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

int div_ceil(int a, int b) { return (a + b - 1) / b; }

ll log10ll(ll n) {
  ll res = 0;
  while (n >= 10) {
    n /= 10LL;
    res++;
  }
  return res;
}

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

namespace Gaussian {
// 標準正規分布の確率密度関数
Unit phi(Unit x) {
  return (1.0 / std::sqrt(2.0 * M_PI)) * std::exp(-0.5 * x * x);
}

// 標準正規分布の累積分布関数
Unit Phi(Unit x) { return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0))); }
} // namespace Gaussian
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

struct Solver {

  double optimization_start_temp;
  double optimization_end_temp;
  Solver() {
    optimization_start_temp = get_env_double("OPTIMIZATION_START_TEMP", 100.0);
    optimization_end_temp = get_env_double("OPTIMIZATION_END_TEMP", 0.01);
  }

  void optimize(Solution &solution, double over_ratio = 1.0) {
    INFO_LOG("# annealing", time_keeper.get_elapsed_ms(), "score",
             solution.score);

    const double check_time_elapsed_ratio = 0.002;
    int check_time_freq = 1;
    const int num_time_sample = 100;

    auto best_score = solution.score;

    auto best_solution = solution;

    auto init_time = time_keeper.get_elapsed_ratio();
    auto now_solution = solution;
    auto now_score = best_score;
    double crt_elapsed_ratio = time_keeper.get_elapsed_ratio();
    int update_cnt = 0;
    int improved_cnt = 0;

    for (int lp = 0;; lp++) {
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
          solution = std::move(best_solution);
          break;
        }
      }

      auto elapsed_ratio =
          (crt_elapsed_ratio - init_time) / ((over_ratio - init_time));
      double temp =
          optimization_start_temp +
          (optimization_end_temp - optimization_start_temp) * elapsed_ratio;
      double diff_threshold = temp * log(randf());

      auto next_score = calc_neighbor_ans(now_solution, now_score,
                                          diff_threshold, crt_elapsed_ratio);

      auto diff = next_score - now_score;
      if (diff < diff_threshold) {
        continue;
      }

      now_score = next_score;
      update_cnt++;

      if (chmax(best_score, now_score)) {
        best_solution = now_solution;
        improved_cnt++;

        DEBUG_LOG("update by annealing!!", best_score, "lp", lp, "update",
                  update_cnt, "elapsed ratio", elapsed_ratio);
      }
    }
  }

  Unit calc_neighbor_ans(Solution &solution, int score, double diff_threshold,
                         double elapsed_ratio) {
    // TODO
    return -UINF;
  }
};