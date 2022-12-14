#ifndef BZLA_SOLVER_ARRAY_ARRAY_SOLVER_H_INCLUDED
#define BZLA_SOLVER_ARRAY_ARRAY_SOLVER_H_INCLUDED

#include "backtrack/unordered_set.h"
#include "backtrack/vector.h"
#include "solver/solver.h"
#include "util/statistics.h"

namespace bzla::array {

class ArraySolver : public Solver
{
 public:
  /** Determine if `term` is a leaf node for the array solver. */
  static bool is_leaf(const Node& term);

  ArraySolver(Env& env, SolverState& state);
  ~ArraySolver();

  void check() override;

  Node value(const Node& term) override;

  void register_term(const Node& term) override;

 private:
  /**
   * Utility class used to store array selects and stores in d_array_models. It
   * provides uniform access to query the corresponding arrays, indices and
   * elements.
   *
   * A Access class is hashed and compared based on the current model value of
   * the read index.
   *
   * @note: This class caches model values and hash values in order to avoid
   *        repeatedly querying and computing the hash values when accessing an
   *        array model.
   */
  class Access
  {
   public:
    Access(const Node& access, SolverState& state);

    /** @return Associated access node. */
    const Node& get() const;

    /** @return Associated element term. */
    const Node& element() const;

    /** @return Associated index term. */
    const Node& index() const;

    /** @return Associated array term. */
    const Node& array() const;

    /** @return Value of associated access node. */
    const Node& element_value() const;

    /** @return Value of read index. */
    const Node& index_value() const;

    /** Compare two access nodes based on d_index_value. */
    bool operator==(const Access& other) const;

    /** Compute hash value based on d_index_value. */
    size_t hash() const;

   private:
    /** Associated access node. */
    Node d_access;
    /** Cached hash value. */
    size_t d_hash;
    /** Value of the access node. */
    Node d_value;
    /** Value of read index. */
    Node d_index_value;
  };

  /** Hash struct for hashing Access. */
  struct HashAccess
  {
    size_t operator()(const Access& access) const { return access.hash(); }
  };

  /** Check theory consistency of access. */
  void check_access(const Node& access);

  /** Check theory consistency of array equality. */
  void check_equality(const Node& eq);

  /**
   * Add congruence lemma for (access a i), (access a j).
   *
   * Lemma: <path conditions> /\ i = j => (access a i) (access a j), where
   * <path conditions> are the conditions along the propagation paths of both
   * access nodes. These are constructed via collect_path_conditions().
   */
  void add_congruence_lemma(const Node& array,
                            const Access& acc1,
                            const Access& acc2);
  /**
   * Add access-store lemma (access (store a i e) j).
   *
   * Lemma: <path conditions> /\ i = j => (access (store a i e) j) = e, where
   * <path conditions> are the conditions along the propagation path of the
   * access. These are constructed via collect_path_conditions().
   */
  void add_access_store_lemma(const Access& acc, const Node& store);

  /**
   * Add access-const array lemma (access ((as const (...) v)) i).
   *
   * Lemma: <path conditions> => (access ((as const (...) v)) i) = v, where
   * <path conditions> are the conditions along the propagation path of the
   * access. These are constructed via collect_path_conditions().
   */
  void add_access_const_array_lemma(const Access& acc, const Node& array);

  /**
   * Add array disequality lemma for a = b.
   *
   * Lemma: a != b => a[k] != b[k] for a fresh k, where a[k] and b[k] act as
   * witnesses for the disequality of the two arrays.
   *
   * @return The disequality witnesses (a[k], b[k]).
   */
  std::pair<Node, Node> add_disequality_lemma(const Node& eq);

  /**
   * Find shortest path from access to array and construct the path conditions
   * required to get there.
   */
  void collect_path_conditions(const Access& access,
                               const Node& array,
                               std::vector<Node>& conditions);

  /**
   * Compute the parents for the array terms in given term.
   *
   * The parents are required for array terms fo doing the upward propagation
   * of access nodes.
   */
  void compute_parents(const Node& term);

  /** Registered array selects. */
  backtrack::vector<Node> d_selects;

  /** Registered array equalities. */
  backtrack::vector<Node> d_equalities;

  /**
   * Array models constructed during check().
   * @note This cache is reset each check() call.
   */
  std::unordered_map<Node, std::unordered_set<Access, HashAccess>>
      d_array_models;

  /**
   * Caches access nodes already checked in check_access().
   * @note This cache is reset each check() call.
   */
  std::unordered_set<Node> d_check_access_cache;

  /**
   * Maps array terms to their array parents, used for upwards propagation.
   * @note This map is computed in compute_parents().
   */
  std::unordered_map<Node, std::vector<Node>> d_parents;
  /** Currently active parents. */
  backtrack::unordered_set<Node> d_active_parents;
  /**
   * Lemma cache for array disequalities. Maps equality to pair of selects,
   * which acts as witnesses for array disequality.
   */
  std::unordered_map<Node, std::pair<Node, Node>> d_disequality_lemma_cache;

  struct Statistics
  {
    Statistics(util::Statistics& stats);
    uint64_t& num_checks;
    uint64_t& num_propagations;
    uint64_t& num_propagations_up;
    uint64_t& num_propagations_down;
    util::HistogramStatistic& num_lemma_size;
    util::TimerStatistic& time_check;
  } d_stats;
};

}  // namespace bzla::array

#endif