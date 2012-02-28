#ifndef LM_SEARCH_HASHED__
#define LM_SEARCH_HASHED__

#include "lm/model_type.hh"
#include "lm/config.hh"
#include "lm/read_arpa.hh"
#include "lm/return.hh"
#include "lm/weights.hh"

#include "util/bit_packing.hh"
#include "util/probing_hash_table.hh"

#include <algorithm>
#include <iostream>
#include <vector>

namespace util { class FilePiece; }

namespace lm {
namespace ngram {
struct Backing;
namespace detail {

inline uint64_t CombineWordHash(uint64_t current, const WordIndex next) {
  uint64_t ret = (current * 8978948897894561157ULL) ^ (static_cast<uint64_t>(1 + next) * 17894857484156487943ULL);
  return ret;
}

#pragma pack(push)
#pragma pack(4)
struct ProbEntry {
  uint64_t key;
  Prob value;
  typedef uint64_t Key;
  typedef Prob Value;
  uint64_t GetKey() const {
    return key;
  }
};

#pragma pack(pop)

class LongestPointer {
  public:
    explicit LongestPointer(const float &to) : to_(&to) {}

    LongestPointer() : to_(NULL) {}

    bool Found() const {
      return to_ != NULL;
    }

    float Prob() const {
      return *to_;
    }

  private:
    const float *to_;
};

template <class Value> class HashedSearch {
  public:
    typedef uint64_t Node;

    typedef typename Value::ProbingProxy UnigramPointer;
    typedef typename Value::ProbingProxy MiddlePointer;
    typedef ::lm::ngram::detail::LongestPointer LongestPointer;

    static const ModelType kModelType = Value::kProbingModelType;
    static const bool kDifferentRest = Value::kDifferentRest;
    static const unsigned int kVersion = 0;

    // TODO: move probing_multiplier here with next binary file format update.  
    static void UpdateConfigFromBinary(int, const std::vector<uint64_t> &, Config &) {}

    static std::size_t Size(const std::vector<uint64_t> &counts, const Config &config) {
      std::size_t ret = Unigram::Size(counts[0]);
      for (unsigned char n = 1; n < counts.size() - 1; ++n) {
        ret += Middle::Size(counts[n], config.probing_multiplier);
      }
      return ret + Longest::Size(counts.back(), config.probing_multiplier);
    }

    uint8_t *SetupMemory(uint8_t *start, const std::vector<uint64_t> &counts, const Config &config);

    template <class Voc> void InitializeFromARPA(const char *file, util::FilePiece &f, const std::vector<uint64_t> &counts, const Config &config, Voc &vocab, Backing &backing);

    void LoadedBinary();

    unsigned char Order() const {
      return middle_.size() + 2;
    }

    typename Value::Weights &UnknownUnigram() { return unigram_.Unknown(); }

    UnigramPointer LookupUnigram(WordIndex word, Node &next, bool &independent_left, uint64_t &extend_left) const {
      extend_left = static_cast<uint64_t>(word);
      next = extend_left;
      UnigramPointer ret(unigram_.Lookup(word));
      independent_left = ret.IndependentLeft();
      return ret;
    }

#pragma GCC diagnostic ignored "-Wuninitialized"
    MiddlePointer Unpack(uint64_t extend_pointer, unsigned char extend_length, Node &node) const {
      node = extend_pointer;
      typename Middle::ConstIterator found;
      bool got = middle_[extend_length - 2].Find(extend_pointer, found);
      assert(got);
      (void)got;
      return MiddlePointer(found->value);
    }

    MiddlePointer LookupMiddle(unsigned char order_minus_2, WordIndex word, Node &node, bool &independent_left, uint64_t &extend_pointer) const {
      node = CombineWordHash(node, word);
      typename Middle::ConstIterator found;
      if (!middle_[order_minus_2].Find(node, found)) {
        independent_left = true;
        return MiddlePointer();
      }
      extend_pointer = node;
      MiddlePointer ret(found->value);
      independent_left = ret.IndependentLeft();
      return ret;
    }

    LongestPointer LookupLongest(WordIndex word, const Node &node) const {
      // Sign bit is always on because longest n-grams do not extend left.  
      typename Longest::ConstIterator found;
      if (!longest_.Find(CombineWordHash(node, word), found)) return LongestPointer();
      return LongestPointer(found->value.prob);
    }

    // Generate a node without necessarily checking that it actually exists.  
    // Optionally return false if it's know to not exist.  
    bool FastMakeNode(const WordIndex *begin, const WordIndex *end, Node &node) const {
      assert(begin != end);
      node = static_cast<Node>(*begin);
      for (const WordIndex *i = begin + 1; i < end; ++i) {
        node = CombineWordHash(node, *i);
      }
      return true;
    }

  private:
    class Unigram {
      public:
        Unigram() {}

        Unigram(void *start, std::size_t /*allocated*/) : unigram_(static_cast<typename Value::Weights*>(start)) {}

        static std::size_t Size(uint64_t count) {
          return (count + 1) * sizeof(ProbBackoff); // +1 for hallucinate <unk>
        }

        const typename Value::Weights &Lookup(WordIndex index) const { return unigram_[index]; }

        typename Value::Weights &Unknown() { return unigram_[0]; }

        void LoadedBinary() {}

        // For building.
        typename Value::Weights *Raw() { return unigram_; }

      private:
        typename Value::Weights *unigram_;
    };

    Unigram unigram_;

    typedef util::ProbingHashTable<typename Value::ProbingEntry, util::IdentityHash> Middle;
    std::vector<Middle> middle_;

    typedef util::ProbingHashTable<ProbEntry, util::IdentityHash> Longest;
    Longest longest_;
};

} // namespace detail
} // namespace ngram
} // namespace lm

#endif // LM_SEARCH_HASHED__
