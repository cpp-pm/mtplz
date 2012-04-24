#ifndef SEARCH_EDGE__
#define SEARCH_EDGE__

#include "search/arity.hh"
#include "search/rule.hh"
#include "search/types.hh"
#include "search/vertex.hh"

#include <queue>

namespace search {

class Context;

class Edge {
  public:
    Edge() {
      end_to_ = to_;
    }

    Rule &InitRule() { return rule_; }

    void Add(Vertex &vertex) {
      assert(end_to_ - to_ < kMaxArity);
      *(end_to_++) = &vertex;
    }

    void FinishedAdding(const Context &) const {}

    const Vertex &GetVertex(std::size_t index) const {
      return *to_[index];
    }

    const Rule &GetRule() const { return rule_; }

  private:
    // Rule and pointers to rule arguments.  
    Rule rule_;

    Vertex *to_[kMaxArity];
    Vertex **end_to_;
};

struct PartialEdge {
  Score score;
  PartialVertex nt[kMaxArity];

  bool operator<(const PartialEdge &other) const {
    return score < other.score;
  }
};

} // namespace search
#endif // SEARCH_EDGE__
