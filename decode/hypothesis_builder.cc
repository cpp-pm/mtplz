#include "decode/hypothesis_builder.hh"

namespace decode {
  
Hypothesis *HypothesisBuilder::BuildHypothesis(const lm::ngram::Right &state, float score) {
  void *hypo = feature_init_.HypothesisLayout().Allocate(pool_);
  feature_init_.HypothesisField()(hypo) = Hypothesis(score);
  feature_init_.LMStateField()(hypo) = state;
  return reinterpret_cast<Hypothesis*>(hypo);
}

Hypothesis *HypothesisBuilder::BuildHypothesis(
    Hypothesis *base,
    const lm::ngram::Right &state,
    float score,
    const Hypothesis *previous,
    std::size_t source_begin,
    std::size_t source_end,
    const TargetPhrase *target) {
  feature_init_.HypothesisField()(base) = Hypothesis(score, previous, source_begin, source_end, target);
  feature_init_.LMStateField()(base) = state;
  return base;
}

Hypothesis *HypothesisBuilder::NextHypothesis(const Hypothesis *previous_hypothesis) {
  void *hypo = feature_init_.HypothesisLayout().Allocate(pool_);
  feature_init_.HypothesisField()(hypo) = Hypothesis(previous_hypothesis);
  return reinterpret_cast<Hypothesis*>(hypo);
}

Hypothesis *HypothesisBuilder::CopyHypothesis(Hypothesis *hypothesis) const {
  std::size_t hypothesis_size = feature_init_.HypothesisLayout().OffsetsBegin();
  void *copy = feature_init_.HypothesisLayout().Allocate(pool_);
  std::memcpy(copy, hypothesis, hypothesis_size);
  return reinterpret_cast<Hypothesis*>(copy);
}

} // namespace decode
