#include <vector>
#include "LM01.h"
#include "moses/ScoreComponentCollection.h"
#include "moses/Hypothesis.h"
#include "FactorCollection.h"
#include "InputFileStream.h"

using namespace std;

namespace Moses
{
int LM01State::Compare(const FFState& other) const
{
  const LM01State &otherState = static_cast<const LM01State&>(other);

  if (m_hash == otherState.m_hash)
    return 0;
  return (m_hash < otherState.m_hash) ? -1 : +1;
}

////////////////////////////////////////////////////////////////
LM01::LM01(const std::string &line)
  :StatefulFeatureFunction(3, line)
  ,m_beginFactor(0)
  ,m_endFactor(0)
  ,m_ignoreUnk(true)
  ,m_minCount(1)
{
  ReadParameters();

  UTIL_THROW_IF2(m_filePath.empty(), "Must specify data directory");
}

void LM01::Load()
{
  FactorCollection &factorCollection = FactorCollection::Instance();

  string line;

  string vocabPath = m_filePath + "/Vocab.dat";
  InputFileStream vocabStrme(vocabPath);

  while (getline(vocabStrme, line)) {
	  vector<string> toks;
	  Tokenize(toks, line);
	  UTIL_THROW_IF2(toks.size() != 2, "Expected 2 columns in vocab file");

	  const Factor *factor = factorCollection.AddFactor(toks[0]);
	  size_t id = Scan<size_t>(toks[1]);

	  m_word2id[factor] = id;
  }

  string dataPath = m_filePath + "/Data.dat";
  InputFileStream dataStrme(dataPath);

  while (getline(dataStrme, line)) {
	  vector<string> toks;
	  Tokenize(toks, line);
	  UTIL_THROW_IF2(toks.size() != 2, "Expected 2 columns in data file");

	  size_t hash = Scan<size_t>(toks[0]);
	  float count = Scan<float>(toks[1]);

	  m_data[hash] = count;
  }

}

void LM01::EvaluateInIsolation(const Phrase &source
    , const TargetPhrase &targetPhrase
    , ScoreComponentCollection &scoreBreakdown
    , ScoreComponentCollection &estimatedFutureScore) const
{}

void LM01::EvaluateWithSourceContext(const InputType &input
    , const InputPath &inputPath
    , const TargetPhrase &targetPhrase
    , const StackVec *stackVec
    , ScoreComponentCollection &scoreBreakdown
    , ScoreComponentCollection *estimatedFutureScore) const
{}

void LM01::EvaluateTranslationOptionListWithSourceContext(const InputType &input
    , const TranslationOptionList &translationOptionList) const
{}

FFState* LM01::EvaluateWhenApplied(
  const Hypothesis& cur_hypo,
  const FFState* prev_state,
  ScoreComponentCollection* accumulator) const
{
  // dense scores
  vector<float> newScores(m_numScoreComponents);
  newScores[0] = 1.5;
  newScores[1] = 0.3;
  newScores[2] = 0.4;
  accumulator->PlusEquals(this, newScores);

  // sparse scores
  accumulator->PlusEquals(this, "sparse-name", 2.4);

  // int targetLen = cur_hypo.GetCurrTargetPhrase().GetSize(); // ??? [UG]
  return new LM01State(0);
}

FFState* LM01::EvaluateWhenApplied(
  const ChartHypothesis& /* cur_hypo */,
  int /* featureID - used to index the state in the previous hypotheses */,
  ScoreComponentCollection* accumulator) const
{
  return new LM01State(0);
}

void LM01::SetParameter(const std::string& key, const std::string& value)
{
  if (key == "begin-factor") {
	  m_beginFactor = Scan<size_t>(value);
  }
  else if (key == "end-factor") {
	  m_endFactor = Scan<size_t>(value);
  }
  else if (key == "path") {
	  m_filePath = value;
  }
  else {
    StatefulFeatureFunction::SetParameter(key, value);
  }
}

}

