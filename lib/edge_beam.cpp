/**************************************************************/
// 差分更新ビームサーチライブラリ (rhoo方式 / post-order tree)
// 履歴木 (各 turn の選択遷移を木で表現) を「葉ごとの post-order
// Action 配列 + 葉境界」で持ち、move_forward/move_backward で
// in-place に状態を差分更新する。
// 前進辺/後退辺のマーカーを持たない点で eijirou方式 (Euler Tour
// ベース) とは異なる。API は eijirou方式の従来版と互換。
//   - dfs の per-entry 分岐除去 (forward walk: revert range → apply range)
//   - tour エントリのスリム化 (Action 単体, type marker なし)
//   - direct_road の LCA ベース一括抽出
// を実現。
//
// Hashを用いた同一盤面除去をする場合は
// Hash, Action, Cost, State を自分で定義、実装して
// using BeamSearchUser = EdgeBeamSearch<Hash, Action, Cost, StateBase>;
// Hashの処理を記述するのが面倒な場合は
// Action, Cost, State を自分で定義、実装して
// using BeamSearchUser = EdgeBeamSearchNoHash<Action, Cost, StateBase>;
// と記述し、
// BeamSearchUser beam_search;
// を用いてビームサーチを実行する。
// Action以外はそれぞれのconceptに準拠している必要がある。
// テンプレートメタプログラミングによって実現しているので、
// 抽象クラスを継承する場合と比べてオーバーヘッドがかからないはず。
// 参考: https://eijirou-kyopro.hatenablog.com/entry/2024/02/01/115639
//       https://trap.jp/post/2920/
// 要件
// ac-liblrary: https://github.com/atcoder/ac-library
// 推奨
// 単にコピペしただけでも使えるが、ymatsuxさんの分割コンパイルツールと合わせると
// ローカルのコードがすっきりしてオススメ
// https://github.com/ymatsux/competitive-programming/tree/main/combiner
/**************************************************************/
#pragma once
#ifndef EDGE_BEAM_HPP
#define EDGE_BEAM_HPP
#include <bits/stdc++.h>
#include <atcoder/segtree>

namespace edge_beam_library
{
    using namespace std;

    // 連想配列
    // Keyにハッシュ関数を適用しない
    // open addressing with linear probing
    // unordered_mapよりも速い
    // nは格納する要素数よりも16倍ほど大きくする
    template <class Key, class T>
    struct HashMap
    {
    public:
        explicit HashMap(uint32_t n)
        {
            if (n % 2 == 0)
            {
                ++n;
            }
            n_ = n;
            valid_.resize(n_, false);
            data_.resize(n_);
        }

        // 戻り値
        // - 存在するならtrue、存在しないならfalse
        // - index
        pair<bool, int> get_index(Key key) const
        {
            Key i = key % n_;
            while (valid_[i])
            {
                if (data_[i].first == key)
                {
                    return {true, (int)i};
                }
                if (++i == n_)
                {
                    i = 0;
                }
            }
            return {false, (int)i};
        }

        // 指定したindexにkeyとvalueを格納する
        void set(int i, Key key, T value)
        {
            valid_[i] = true;
            data_[i] = {key, value};
        }

        // 指定したindexのvalueを返す
        T get(int i) const
        {
            assert(valid_[i]);
            return data_[i].second;
        }

        void clear()
        {
            fill(valid_.begin(), valid_.end(), false);
        }

    private:
        uint32_t n_;
        vector<bool> valid_;
        vector<pair<Key, T>> data_;
    };

    template <typename HashType>
    concept HashConcept = requires(HashType hash) {
        { std::is_unsigned_v<HashType> };
    };
    template <typename CostType>
    concept CostConcept = requires(CostType cost) {
        { std::is_arithmetic_v<CostType> };
    };

    template <typename StateType, typename HashType, typename CostType, typename ActionType, typename SelectorType>
    concept StateConcept = HashConcept<HashType> &&
                           CostConcept<CostType> &&
                           requires(StateType state, SelectorType selector) {
                               { state.expand(std::declval<int>(), selector) } -> same_as<void>;
                               { state.move_forward(std::declval<ActionType>()) } -> same_as<void>;
                               { state.move_backward(std::declval<ActionType>()) } -> same_as<void>;
                               { state.make_initial_node() } -> same_as<pair<CostType, HashType>>;
                           };

    template <HashConcept Hash, typename Action, CostConcept Cost, template <typename> class State>
    struct EdgeBeamSearch
    {
        // ビームサーチの設定
        struct Config
        {
            int max_turn;
            size_t beam_width;          // 初期 beam_width
            size_t max_beam_width = 0;  // 内部配列の確保サイズ (0 なら beam_width と同じ)
            size_t tour_capacity;
            uint32_t hash_map_capacity;
            // 実行可能解が見つかったらすぐに返すかどうか
            bool return_finished_immediately;
            // 時間制限チェック用コールバック (nullptr なら無制限)
            function<bool()> is_time_over = nullptr;
            // 動的 beam_width コールバック
            function<size_t(int turn)> get_beam_width = nullptr;
        };

        // 展開するノードの候補を表す構造体
        struct Candidate
        {
            Action action;
            Cost cost;
            Hash hash;
            int parent;

            Candidate(Action action, Cost cost, Hash hash, int parent) : action(action),
                                                                         cost(cost),
                                                                         hash(hash),
                                                                         parent(parent) {}
        };

        // ノードの候補から実際に追加するものを選ぶクラス
        // ビーム幅の個数だけ、評価がよいものを選ぶ
        // ハッシュ値が一致したものについては、評価がよいほうのみを残す
        class Selector
        {
        public:
            explicit Selector(const Config &config) : hash_to_index_(config.hash_map_capacity)
            {
                beam_width = config.beam_width;
                max_beam_width_ = config.max_beam_width > 0 ? config.max_beam_width : config.beam_width;
                candidates_.reserve(max_beam_width_);
                full_ = false;

                costs_.resize(max_beam_width_);
                for (size_t i = 0; i < max_beam_width_; ++i)
                {
                    costs_[i] = {0, (int)i};
                }
            }

            // beam_width を動的に変更 (clear 後に呼ぶこと)
            void set_beam_width(size_t new_bw)
            {
                beam_width = min(new_bw, max_beam_width_);
            }

            // 候補を追加する
            void push(const Action &action, const Cost &cost, const Hash &hash, int parent, bool finished)
            {
                Candidate candidate(action, cost, hash, parent);
                if (finished)
                {
                    finished_candidates_.emplace_back(candidate);
                    return;
                }
                if (full_ && cost >= st_.prod(0, beam_width).first)
                {
                    return;
                }
                auto [valid, i] = hash_to_index_.get_index(candidate.hash);

                if (valid)
                {
                    int j = hash_to_index_.get(i);
                    if (candidate.hash == candidates_[j].hash)
                    {
                        if (full_)
                        {
                            if (cost < st_.get(j).first)
                            {
                                candidates_[j] = candidate;
                                st_.set(j, {cost, j});
                            }
                        }
                        else
                        {
                            if (cost < costs_[j].first)
                            {
                                candidates_[j] = candidate;
                                costs_[j].first = cost;
                            }
                        }
                        return;
                    }
                }
                if (full_)
                {
                    int j = st_.prod(0, beam_width).second;
                    hash_to_index_.set(i, candidate.hash, j);
                    candidates_[j] = candidate;
                    st_.set(j, {cost, j});
                }
                else
                {
                    int j = candidates_.size();
                    hash_to_index_.set(i, candidate.hash, j);
                    candidates_.emplace_back(candidate);
                    costs_[j].first = cost;

                    if (candidates_.size() == beam_width)
                    {
                        full_ = true;
                        st_ = MaxSegtree(costs_);
                    }
                }
            }

            const vector<Candidate> &select() const { return candidates_; }
            bool have_finished() const { return !finished_candidates_.empty(); }
            vector<Candidate> get_finished_candidates() const { return finished_candidates_; }

            Candidate calculate_best_candidate() const
            {
                if (full_)
                {
                    size_t best = 0;
                    for (size_t i = 0; i < beam_width; ++i)
                    {
                        if (st_.get(i).first < st_.get(best).first)
                        {
                            best = i;
                        }
                    }
                    return candidates_[best];
                }
                else
                {
                    size_t best = 0;
                    for (size_t i = 0; i < candidates_.size(); ++i)
                    {
                        if (costs_[i].first < costs_[best].first)
                        {
                            best = i;
                        }
                    }
                    return candidates_[best];
                }
            }

            void clear()
            {
                candidates_.clear();
                hash_to_index_.clear();
                full_ = false;
            }

            void clear_finished_candidates() { finished_candidates_.clear(); }

        private:
            using MaxSegtree = atcoder::segtree<
                pair<Cost, int>,
                [](pair<Cost, int> a, pair<Cost, int> b)
                { return a.first >= b.first ? a : b; },
                []()
                { return make_pair(numeric_limits<Cost>::min(), -1); }>;

            size_t beam_width;
            size_t max_beam_width_;
            vector<Candidate> candidates_;
            HashMap<Hash, int> hash_to_index_;
            bool full_;
            vector<pair<Cost, int>> costs_;
            MaxSegtree st_;
            vector<Candidate> finished_candidates_;
        };

        // 履歴木を post-order Action 配列で管理するクラス (rhoo方式)
        // tour_   : post-order に並べた Action 配列 (各 leaf の LCA からの増分を連結)
        // leaves_ : 境界配列。leaves_[i+1] - leaves_[i] = leaf i の LCA(i-1,i) からの descent 長
        //           leaves_.size() = L + 1 (L は leaf 数)
        // turn_   : direct_road_ の終端から現在の leaf 群までの深さ
        // trace_indices_ : dfs/update walk 中の現在パスを tour_ への index で保持
        // direct_road_   : 全 leaf 共通プレフィクス
        class Tree
        {
        public:
            explicit Tree(const State<Selector> &state, const Config &config) : state_(state)
            {
                size_t max_bw = config.max_beam_width > 0 ? config.max_beam_width : config.beam_width;
                tour_.reserve(config.tour_capacity);
                next_tour_.reserve(config.tour_capacity);
                leaves_.reserve(max_bw + 2);
                next_leaves_.reserve(max_bw + 2);
                trace_indices_.reserve(config.max_turn + 1);
                cand_buckets_.assign(max_bw, {});
                turn_ = 0;
                leaves_.push_back(0);
            }

            // 状態を更新しながら leaf を順に訪問し、各 leaf で expand を呼ぶ
            void dfs(Selector &selector)
            {
                if (turn_ == 0)
                {
                    // 最初のターン: 初期 state から expand
                    state_.make_initial_node();
                    state_.expand(0, selector);
                    return;
                }
                int L = (int)leaves_.size() - 1;
                if ((int)trace_indices_.size() < turn_)
                    trace_indices_.assign(turn_, 0);

                int curr_depth = 0;
                for (int k = 0; k < L; k++)
                {
                    int range = leaves_[k + 1] - leaves_[k];
                    // LCA(prev, k) の深さは turn_ - range なので、curr_depth - (turn_ - range) 個 revert
                    int revert_amount = curr_depth - (turn_ - range);
                    for (int i = 0; i < revert_amount; i++)
                    {
                        state_.move_backward(tour_[trace_indices_[curr_depth - 1 - i]]);
                    }
                    curr_depth -= revert_amount;
                    // tour_[leaves_[k] .. leaves_[k+1]] を順に apply
                    for (int j = 0; j < range; j++)
                    {
                        int ti = leaves_[k] + j;
                        trace_indices_[curr_depth + j] = ti;
                        state_.move_forward(tour_[ti]);
                    }
                    curr_depth = turn_;
                    state_.expand(k, selector);
                }
                // root に戻す
                for (int d = curr_depth - 1; d >= 0; d--)
                {
                    state_.move_backward(tour_[trace_indices_[d]]);
                }
            }

            // 木を更新する: candidates から次世代の tour/leaves を構築
            void update(const vector<Candidate> &candidates)
            {
                int L = (int)leaves_.size() - 1;

                // candidates を parent leaf 別にバケット
                size_t needed = max((size_t)max(L, 1), (size_t)1);
                if (cand_buckets_.size() < needed)
                    cand_buckets_.resize(needed);
                for (size_t i = 0; i < candidates.size(); i++)
                {
                    cand_buckets_[candidates[i].parent].push_back((int)i);
                }

                next_tour_.clear();
                next_leaves_.clear();
                next_leaves_.push_back(0);

                if (turn_ == 0)
                {
                    // 初期 expand 後: 全 cand は parent=0
                    for (int cidx : cand_buckets_[0])
                    {
                        next_tour_.push_back(candidates[cidx].action);
                        next_leaves_.push_back((int)next_tour_.size());
                    }
                    cand_buckets_[0].clear();
                }
                else
                {
                    if ((int)trace_indices_.size() < turn_)
                        trace_indices_.assign(turn_, 0);

                    int curr_depth = 0;
                    int prev_active_parent = -1;
                    int max_range_since_prev_active = 0;

                    for (int k = 0; k < L; k++)
                    {
                        int range = leaves_[k + 1] - leaves_[k];
                        int revert_amount = curr_depth - (turn_ - range);
                        for (int i = 0; i < revert_amount; i++)
                        {
                            state_.move_backward(tour_[trace_indices_[curr_depth - 1 - i]]);
                        }
                        curr_depth -= revert_amount;
                        for (int j = 0; j < range; j++)
                        {
                            int ti = leaves_[k] + j;
                            trace_indices_[curr_depth + j] = ti;
                            state_.move_forward(tour_[ti]);
                        }
                        curr_depth = turn_;

                        // LCA(prev_active, k) の depth 計算用に max_range を更新
                        if (prev_active_parent >= 0)
                        {
                            if (range > max_range_since_prev_active)
                                max_range_since_prev_active = range;
                        }

                        if (cand_buckets_[k].empty())
                            continue;

                        // 新 leaf 群を発行
                        int lca_depth;
                        if (prev_active_parent < 0)
                        {
                            lca_depth = 0; // 仮想 root
                        }
                        else
                        {
                            lca_depth = turn_ - max_range_since_prev_active;
                        }

                        bool first = true;
                        for (int cidx : cand_buckets_[k])
                        {
                            if (first)
                            {
                                // 最初の cand: LCA から現在 (turn_) までの trace + cand.action
                                for (int d = lca_depth; d < turn_; d++)
                                {
                                    next_tour_.push_back(tour_[trace_indices_[d]]);
                                }
                                next_tour_.push_back(candidates[cidx].action);
                                first = false;
                            }
                            else
                            {
                                // 同 parent の続き: cand.action のみ (LCA = parent 自身, depth turn_)
                                next_tour_.push_back(candidates[cidx].action);
                            }
                            next_leaves_.push_back((int)next_tour_.size());
                        }

                        prev_active_parent = k;
                        max_range_since_prev_active = 0;
                        cand_buckets_[k].clear();
                    }

                    // root に戻す
                    for (int d = curr_depth - 1; d >= 0; d--)
                    {
                        state_.move_backward(tour_[trace_indices_[d]]);
                    }
                }

                int new_turn = turn_ + 1;

                // direct_road 抽出: 全 leaf 共通プレフィクス = LCA(全 leaves) の深さ分
                int newL = (int)next_leaves_.size() - 1;
                if (newL > 0)
                {
                    int max_range_others = 0;
                    for (int i = 1; i < newL; i++)
                    {
                        int r = next_leaves_[i + 1] - next_leaves_[i];
                        if (r > max_range_others)
                            max_range_others = r;
                    }
                    // newL == 1 の場合: 単一 leaf は全部 direct_road にできる
                    int shared = new_turn - max_range_others;
                    if (shared > 0)
                    {
                        for (int i = 0; i < shared; i++)
                        {
                            direct_road_.push_back(next_tour_[i]);
                            state_.move_forward(next_tour_[i]);
                        }
                        // next_tour_ から先頭 shared 個を除去
                        next_tour_.erase(next_tour_.begin(), next_tour_.begin() + shared);
                        for (int i = 1; i < (int)next_leaves_.size(); i++)
                        {
                            next_leaves_[i] -= shared;
                        }
                        new_turn -= shared;
                    }
                }

                swap(tour_, next_tour_);
                swap(leaves_, next_leaves_);
                turn_ = new_turn;
            }

            // 根からのパスを取得する
            // parent_leaf は現在世代の leaf index
            vector<Action> calculate_path(int parent_leaf, int turn_hint) const
            {
                vector<Action> ret;
                ret.reserve((size_t)max(turn_hint, 0));
                ret.insert(ret.end(), direct_road_.begin(), direct_road_.end());

                if (turn_ == 0)
                {
                    return ret;
                }

                // forward walk して parent_leaf 到達時の trace を構築
                vector<int> trace_local(turn_, 0);
                int curr_depth = 0;
                for (int k = 0; k <= parent_leaf; k++)
                {
                    int range = leaves_[k + 1] - leaves_[k];
                    curr_depth = turn_ - range;
                    for (int j = 0; j < range; j++)
                    {
                        trace_local[curr_depth + j] = leaves_[k] + j;
                    }
                    curr_depth = turn_;
                }
                for (int d = 0; d < turn_; d++)
                {
                    ret.push_back(tour_[trace_local[d]]);
                }
                return ret;
            }

        private:
            State<Selector> state_;
            vector<Action> tour_;
            vector<int> leaves_;
            vector<int> trace_indices_;
            vector<Action> direct_road_;
            vector<Action> next_tour_;
            vector<int> next_leaves_;
            vector<vector<int>> cand_buckets_;
            int turn_;
        };

        // ビームサーチを行う関数
        vector<Action> beam_search(const Config &config, const State<Selector> &state)
        {
            Tree tree(state, config);

            Selector selector(config);

            Cost best_cost = numeric_limits<Cost>::max();
            vector<Action> best_ret;
            for (int turn = 0; turn < config.max_turn; ++turn)
            {
                if (config.is_time_over && config.is_time_over())
                {
                    return best_ret;
                }

                tree.dfs(selector);

                if (selector.have_finished())
                {
                    if (config.return_finished_immediately)
                    {
                        Candidate candidate = selector.get_finished_candidates()[0];
                        vector<Action> ret = tree.calculate_path(candidate.parent, turn + 1);
                        ret.push_back(candidate.action);
                        return ret;
                    }
                    else
                    {
                        for (auto candidate : selector.get_finished_candidates())
                        {
                            vector<Action> ret = tree.calculate_path(candidate.parent, turn + 1);
                            ret.push_back(candidate.action);
                            if (candidate.cost < best_cost)
                            {
                                best_cost = candidate.cost;
                                best_ret = ret;
                            }
                        }
                    }
                    selector.clear_finished_candidates();
                }
                if (selector.select().empty())
                {
                    return best_ret;
                }

                if (turn == config.max_turn - 1)
                {
                    Candidate best_candidate = selector.calculate_best_candidate();
                    vector<Action> ret = tree.calculate_path(best_candidate.parent, turn + 1);
                    ret.push_back(best_candidate.action);
                    return ret;
                }

                tree.update(selector.select());

                selector.clear();

                if (config.get_beam_width)
                {
                    size_t new_bw = config.get_beam_width(turn + 1);
                    selector.set_beam_width(new_bw);
                }
            }

            return best_ret;
        }

        static_assert(StateConcept<State<Selector>, Hash, Cost, Action, Selector>,
                      "State template must satisfy StateConcept with BeamSearch::Selector");

    }; // EdgeBeamSearch

    template <typename StateType, typename CostType, typename ActionType, typename SelectorType>
    concept StateConceptNoHash =
        CostConcept<CostType> &&
        requires(StateType state, SelectorType selector) {
            { state.expand(std::declval<int>(), selector) } -> same_as<void>;
            { state.move_forward(std::declval<ActionType>()) } -> same_as<void>;
            { state.move_backward(std::declval<ActionType>()) } -> same_as<void>;
            { state.make_initial_node() } -> CostConcept;
        };

    template <typename Action, CostConcept Cost, template <typename> class State>
    struct EdgeBeamSearchNoHash
    {
        struct Config
        {
            int max_turn;
            size_t beam_width;
            size_t max_beam_width = 0;
            size_t tour_capacity;
            bool return_finished_immediately;
            function<bool()> is_time_over = nullptr;
            function<size_t(int turn)> get_beam_width = nullptr;
        };

        struct Candidate
        {
            Action action;
            Cost cost;
            int parent;

            Candidate(Action action, Cost cost, int parent) : action(action),
                                                              cost(cost),
                                                              parent(parent) {}
        };

        class Selector
        {
        public:
            explicit Selector(const Config &config)
            {
                beam_width = config.beam_width;
                max_beam_width_ = config.max_beam_width > 0 ? config.max_beam_width : config.beam_width;
                candidates_.reserve(max_beam_width_);
                full_ = false;

                costs_.resize(max_beam_width_);
                for (size_t i = 0; i < max_beam_width_; ++i)
                {
                    costs_[i] = {0, (int)i};
                }
            }

            void set_beam_width(size_t new_bw)
            {
                beam_width = min(new_bw, max_beam_width_);
            }

            void push(const Action &action, const Cost &cost, int parent, bool finished)
            {
                Candidate candidate(action, cost, parent);
                if (finished)
                {
                    finished_candidates_.emplace_back(candidate);
                    return;
                }
                if (full_ && cost >= st_.prod(0, beam_width).first)
                {
                    return;
                }
                if (full_)
                {
                    int j = st_.prod(0, beam_width).second;
                    candidates_[j] = candidate;
                    st_.set(j, {cost, j});
                }
                else
                {
                    int j = candidates_.size();
                    candidates_.emplace_back(candidate);
                    costs_[j].first = cost;

                    if (candidates_.size() == beam_width)
                    {
                        full_ = true;
                        st_ = MaxSegtree(costs_);
                    }
                }
            }

            const vector<Candidate> &select() const { return candidates_; }
            bool have_finished() const { return !finished_candidates_.empty(); }
            vector<Candidate> get_finished_candidates() const { return finished_candidates_; }

            Candidate calculate_best_candidate() const
            {
                if (full_)
                {
                    size_t best = 0;
                    for (size_t i = 0; i < beam_width; ++i)
                    {
                        if (st_.get(i).first < st_.get(best).first)
                        {
                            best = i;
                        }
                    }
                    return candidates_[best];
                }
                else
                {
                    size_t best = 0;
                    for (size_t i = 0; i < candidates_.size(); ++i)
                    {
                        if (costs_[i].first < costs_[best].first)
                        {
                            best = i;
                        }
                    }
                    return candidates_[best];
                }
            }

            void clear()
            {
                candidates_.clear();
                full_ = false;
            }

            void clear_finished_candidates() { finished_candidates_.clear(); }

        private:
            using MaxSegtree = atcoder::segtree<
                pair<Cost, int>,
                [](pair<Cost, int> a, pair<Cost, int> b)
                { return a.first >= b.first ? a : b; },
                []()
                { return make_pair(numeric_limits<Cost>::min(), -1); }>;

            size_t beam_width;
            size_t max_beam_width_;
            vector<Candidate> candidates_;
            bool full_;
            vector<pair<Cost, int>> costs_;
            MaxSegtree st_;
            vector<Candidate> finished_candidates_;
        };

        class Tree
        {
        public:
            explicit Tree(const State<Selector> &state, const Config &config) : state_(state)
            {
                size_t max_bw = config.max_beam_width > 0 ? config.max_beam_width : config.beam_width;
                tour_.reserve(config.tour_capacity);
                next_tour_.reserve(config.tour_capacity);
                leaves_.reserve(max_bw + 2);
                next_leaves_.reserve(max_bw + 2);
                trace_indices_.reserve(config.max_turn + 1);
                cand_buckets_.assign(max_bw, {});
                turn_ = 0;
                leaves_.push_back(0);
            }

            void dfs(Selector &selector)
            {
                if (turn_ == 0)
                {
                    state_.make_initial_node();
                    state_.expand(0, selector);
                    return;
                }
                int L = (int)leaves_.size() - 1;
                if ((int)trace_indices_.size() < turn_)
                    trace_indices_.assign(turn_, 0);

                int curr_depth = 0;
                for (int k = 0; k < L; k++)
                {
                    int range = leaves_[k + 1] - leaves_[k];
                    int revert_amount = curr_depth - (turn_ - range);
                    for (int i = 0; i < revert_amount; i++)
                    {
                        state_.move_backward(tour_[trace_indices_[curr_depth - 1 - i]]);
                    }
                    curr_depth -= revert_amount;
                    for (int j = 0; j < range; j++)
                    {
                        int ti = leaves_[k] + j;
                        trace_indices_[curr_depth + j] = ti;
                        state_.move_forward(tour_[ti]);
                    }
                    curr_depth = turn_;
                    state_.expand(k, selector);
                }
                for (int d = curr_depth - 1; d >= 0; d--)
                {
                    state_.move_backward(tour_[trace_indices_[d]]);
                }
            }

            void update(const vector<Candidate> &candidates)
            {
                int L = (int)leaves_.size() - 1;

                size_t needed = max((size_t)max(L, 1), (size_t)1);
                if (cand_buckets_.size() < needed)
                    cand_buckets_.resize(needed);
                for (size_t i = 0; i < candidates.size(); i++)
                {
                    cand_buckets_[candidates[i].parent].push_back((int)i);
                }

                next_tour_.clear();
                next_leaves_.clear();
                next_leaves_.push_back(0);

                if (turn_ == 0)
                {
                    for (int cidx : cand_buckets_[0])
                    {
                        next_tour_.push_back(candidates[cidx].action);
                        next_leaves_.push_back((int)next_tour_.size());
                    }
                    cand_buckets_[0].clear();
                }
                else
                {
                    if ((int)trace_indices_.size() < turn_)
                        trace_indices_.assign(turn_, 0);

                    int curr_depth = 0;
                    int prev_active_parent = -1;
                    int max_range_since_prev_active = 0;

                    for (int k = 0; k < L; k++)
                    {
                        int range = leaves_[k + 1] - leaves_[k];
                        int revert_amount = curr_depth - (turn_ - range);
                        for (int i = 0; i < revert_amount; i++)
                        {
                            state_.move_backward(tour_[trace_indices_[curr_depth - 1 - i]]);
                        }
                        curr_depth -= revert_amount;
                        for (int j = 0; j < range; j++)
                        {
                            int ti = leaves_[k] + j;
                            trace_indices_[curr_depth + j] = ti;
                            state_.move_forward(tour_[ti]);
                        }
                        curr_depth = turn_;

                        if (prev_active_parent >= 0)
                        {
                            if (range > max_range_since_prev_active)
                                max_range_since_prev_active = range;
                        }

                        if (cand_buckets_[k].empty())
                            continue;

                        int lca_depth;
                        if (prev_active_parent < 0)
                        {
                            lca_depth = 0;
                        }
                        else
                        {
                            lca_depth = turn_ - max_range_since_prev_active;
                        }

                        bool first = true;
                        for (int cidx : cand_buckets_[k])
                        {
                            if (first)
                            {
                                for (int d = lca_depth; d < turn_; d++)
                                {
                                    next_tour_.push_back(tour_[trace_indices_[d]]);
                                }
                                next_tour_.push_back(candidates[cidx].action);
                                first = false;
                            }
                            else
                            {
                                next_tour_.push_back(candidates[cidx].action);
                            }
                            next_leaves_.push_back((int)next_tour_.size());
                        }

                        prev_active_parent = k;
                        max_range_since_prev_active = 0;
                        cand_buckets_[k].clear();
                    }

                    for (int d = curr_depth - 1; d >= 0; d--)
                    {
                        state_.move_backward(tour_[trace_indices_[d]]);
                    }
                }

                int new_turn = turn_ + 1;

                int newL = (int)next_leaves_.size() - 1;
                if (newL > 0)
                {
                    int max_range_others = 0;
                    for (int i = 1; i < newL; i++)
                    {
                        int r = next_leaves_[i + 1] - next_leaves_[i];
                        if (r > max_range_others)
                            max_range_others = r;
                    }
                    int shared = new_turn - max_range_others;
                    if (shared > 0)
                    {
                        for (int i = 0; i < shared; i++)
                        {
                            direct_road_.push_back(next_tour_[i]);
                            state_.move_forward(next_tour_[i]);
                        }
                        next_tour_.erase(next_tour_.begin(), next_tour_.begin() + shared);
                        for (int i = 1; i < (int)next_leaves_.size(); i++)
                        {
                            next_leaves_[i] -= shared;
                        }
                        new_turn -= shared;
                    }
                }

                swap(tour_, next_tour_);
                swap(leaves_, next_leaves_);
                turn_ = new_turn;
            }

            vector<Action> calculate_path(int parent_leaf, int turn_hint) const
            {
                vector<Action> ret;
                ret.reserve((size_t)max(turn_hint, 0));
                ret.insert(ret.end(), direct_road_.begin(), direct_road_.end());

                if (turn_ == 0)
                {
                    return ret;
                }

                vector<int> trace_local(turn_, 0);
                int curr_depth = 0;
                for (int k = 0; k <= parent_leaf; k++)
                {
                    int range = leaves_[k + 1] - leaves_[k];
                    curr_depth = turn_ - range;
                    for (int j = 0; j < range; j++)
                    {
                        trace_local[curr_depth + j] = leaves_[k] + j;
                    }
                    curr_depth = turn_;
                }
                for (int d = 0; d < turn_; d++)
                {
                    ret.push_back(tour_[trace_local[d]]);
                }
                return ret;
            }

        private:
            State<Selector> state_;
            vector<Action> tour_;
            vector<int> leaves_;
            vector<int> trace_indices_;
            vector<Action> direct_road_;
            vector<Action> next_tour_;
            vector<int> next_leaves_;
            vector<vector<int>> cand_buckets_;
            int turn_;
        };

        vector<Action> beam_search(const Config &config, const State<Selector> &state)
        {
            Tree tree(state, config);
            Selector selector(config);

            Cost best_cost = numeric_limits<Cost>::max();
            vector<Action> best_ret;
            for (int turn = 0; turn < config.max_turn; ++turn)
            {
                if (config.is_time_over && config.is_time_over())
                {
                    return best_ret;
                }

                tree.dfs(selector);

                if (selector.have_finished())
                {
                    if (config.return_finished_immediately)
                    {
                        Candidate candidate = selector.get_finished_candidates()[0];
                        vector<Action> ret = tree.calculate_path(candidate.parent, turn + 1);
                        ret.push_back(candidate.action);
                        return ret;
                    }
                    else
                    {
                        for (auto candidate : selector.get_finished_candidates())
                        {
                            vector<Action> ret = tree.calculate_path(candidate.parent, turn + 1);
                            ret.push_back(candidate.action);
                            if (candidate.cost < best_cost)
                            {
                                best_cost = candidate.cost;
                                best_ret = ret;
                            }
                        }
                    }
                    selector.clear_finished_candidates();
                }
                if (selector.select().empty())
                {
                    return best_ret;
                }

                if (turn == config.max_turn - 1)
                {
                    Candidate best_candidate = selector.calculate_best_candidate();
                    vector<Action> ret = tree.calculate_path(best_candidate.parent, turn + 1);
                    ret.push_back(best_candidate.action);
                    return ret;
                }

                tree.update(selector.select());

                selector.clear();

                if (config.get_beam_width)
                {
                    size_t new_bw = config.get_beam_width(turn + 1);
                    selector.set_beam_width(new_bw);
                }
            }

            return best_ret;
        }

        static_assert(StateConceptNoHash<State<Selector>, Cost, Action, Selector>,
                      "State template must satisfy StateConcept with BeamSearch::Selector");

    }; // EdgeBeamSearchNoHash

} // namespace edge_beam_library
using namespace edge_beam_library;
#endif
