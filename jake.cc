#include "jake.h"

// Rake implemented with slight modifications following along:
// https://www.researchgate.net/publication/227988510_Automatic_Keyword_Extraction_from_Individual_Documents

// RAKE, short for Rapid Automatic Keyword Extraction, is a domain independent keyword extraction 
// algorithm which tries to determine key phrases in a body of text by analyzing the frequency of word appearance
// and its co-occurance with other words in the text.

char Jake::to_lower(char c) {
    if (c <= 'Z' && c >= 'A') {
        return c - ('Z' - 'z');
    }
    return c;
}

class Jake::PhraseCompare {
  public:
    bool operator() (std::pair<std::string, int> a, std::pair<std::string, int> b) {
      return a.second > b.second;
    }
};

Jake::Jake(std::string stopwords_file_name, std::string document_name) : stopwords_file_name{stopwords_file_name},
                                                                      document_name{document_name} {
  // Get stopwords
  std::string line;
  std::ifstream stopwordsFile(this->stopwords_file_name);

  if (stopwordsFile)  {
    while (std::getline(stopwordsFile, line)) {
      this->stopwords.insert(line);
    }
    stopwordsFile.close();
  }


  // Get document body
  std::string word;
  std::ifstream documentFile(this->document_name);
  while (documentFile >> word) {
    for (auto it = word.begin(); it != word.end(); it++) {
      // Clean up the word
      word.erase(std::remove_if(word.begin(), 
                                word.end(), 
                                [](char c) {
                                  return !(isalpha(c) || c == '\'' || c == '-' || c == ',' || c == '!' || c == '.' || c == '?'); 
                                }), 
                word.end());
    }
    this->document.emplace_back(word);
  }
}

std::vector<std::string> Jake::get_scored_phrases(unsigned num) {
  std::vector<std::string> result;
  if (num > this->scored_phrases.size()) {
    return {};
  } else {
    for (unsigned i = 0; i < num; i++) {
      result.emplace_back(this->scored_phrases[i].first);
    }
  }
  return result;
}

void Jake::process_text() {
  // Isolate candidate phrases
  std::string phrase = "";
  std::vector<std::string> phrases;
  std::unordered_map<std::string, int> candidate_indices; // Maps a number to each candidate word, used for computing co-occurences
  int candidate_count = 0;
  for (auto it = this->document.begin(); it != this->document.end(); it++) {
    std::string word = *it;
    std::string word_lower = *it;
    std::transform(word_lower.begin(), word_lower.end(), word_lower.begin(), this->to_lower);
    auto is_stopword = this->stopwords.find(word_lower);
    if (is_stopword != this->stopwords.end()) {
      if (phrase != "") {
        phrases.emplace_back(phrase);
      }
      phrase = "";
    } else if (word.back() == '!' || word.back() == '?' || word.back() == '.' || word.back() == ',') {
      word.pop_back();
      word_lower.pop_back();
      if (phrase != "") {
        phrase += " ";
      }
      if (candidate_indices.count(word_lower) == 0) {
        candidate_indices.insert(make_pair(word_lower, candidate_count));
        candidate_count++;
      }
      phrase += word;
      phrases.emplace_back(phrase);
      phrase = "";
    } else {
      if (phrase != "") {
        phrase += " ";
      }
      if (candidate_indices.count(word_lower) == 0) {
        candidate_indices.insert(make_pair(word_lower, candidate_count));
        candidate_count++;
      }
      phrase += word;
    }
  }

  // Compute a co-occurences graph
  // Represented as a 2D-array
  // Each keyword is associated with an index, graph[i][i] is a strict count of occurences of word at index i
  // graph[i][j] is a count of word i followed by word j
  std::vector<std::vector<int>> matrix (candidate_count, std::vector<int>(candidate_count, 0));
  for (auto it = phrases.begin(); it != phrases.end(); it++) {
    std::istringstream iss(*it);
    std::string currtoken = "";
    std::string prevtoken = "";
    while (iss >> currtoken) {
      std::transform(currtoken.begin(), currtoken.end(), currtoken.begin(), this->to_lower);
      auto currtoken_index = candidate_indices.find(currtoken);
      auto prevtoken_index = candidate_indices.find(prevtoken);
      if (prevtoken != "" && prevtoken_index != candidate_indices.end()) {
        matrix[prevtoken_index->second][currtoken_index->second]++;
      }
      matrix[currtoken_index->second][currtoken_index->second]++;
      prevtoken = currtoken;
    }
  }

  // Compute word scores from co-occurence graph
  std::vector<int> degrees (candidate_count, 0);  // freq(w) is matrix[w_index][w_index]
  std::vector<int> frequencies (candidate_count, 0); // deg(w) is sum of all matrix[w_index][n] and matrix[n][w_index] minus matrix[w_index][w_index]
  std::vector<double> word_scores (candidate_count, 0); // word_score[i] is the score for word with index i

  for (int i = 0; i < candidate_count; i++) {
    for (int j = 0; j < candidate_count; j++) {
      if (i != j) {
        degrees[i] += matrix[i][j];
        degrees[j] += matrix[i][j];
      } else {
        frequencies[i] = matrix[i][j];
      }
    }
  }

  for (auto it = candidate_indices.begin(); it != candidate_indices.end(); it++) {
    int index = it->second;
    word_scores[index] = (double) degrees[index]/frequencies[index]; // Frequency is never 0
  }

  // -------------------------------------------------------------------------------------------
  // NOTE:
  // Normal RAKE calculates word scores using frequency and degree of each word (explained below).
  // Seemed weird that it didn't take into account *which* words it co-occured with, only that it
  // co-occurred with non-stopwords.
  // -------------------------------------------------------------------------------------------
  std::vector<std::pair<int, int>> co_word_scores (candidate_count, std::make_pair<int, int>(0, 0));
  for (int i = 0; i < candidate_count; i++) {
    for (int j = 0; j < candidate_count; j++) {
      if ((i != j) && (matrix[i][j] != 0)) {
        co_word_scores[i].first += matrix[i][j];
        co_word_scores[i].second += 1;
        co_word_scores[j].first += matrix[i][j];
        co_word_scores[j].second += 1;
      }
    }
  }

  for (auto it = candidate_indices.begin(); it != candidate_indices.end(); it++) {
    int index = it->second;
    if (co_word_scores[index].second != 0) {
      word_scores[index] += (double) co_word_scores[index].first/co_word_scores[index].second;
    }
  }

  std::vector<double> sorted_word_scores = word_scores;
  std::nth_element(sorted_word_scores.begin(),
                    sorted_word_scores.begin() + (candidate_count/2),
                    sorted_word_scores.end());
  int median = sorted_word_scores[candidate_count/2];

  // Assign each phrase a score, based on the sum of its member words' scores
  // -------------------------------------------------------------------------------------------
  // NOTE:
  // Normal RAKE inherently favors longer phrases ... counterbalance by subtracting median
  // Only punishes super long phrases with lots of low-scoring words
  // eg. Really long phrase with low scoring words
  // -------------------------------------------------------------------------------------------
  this->scored_phrases = {};
  for (auto it = phrases.begin(); it != phrases.end(); it++) {
    std::istringstream iss(*it);
    std::string word = "";
    double phrase_score = 0;
    while (iss >> word) {
      std::transform(word.begin(), word.end(), word.begin(), this->to_lower);
      int index = candidate_indices.find(word)->second;
      phrase_score += word_scores[index];
      phrase_score -= median;
    }
    this->scored_phrases.emplace_back(std::make_pair(*it, phrase_score));
  }

  std::sort(this->scored_phrases.begin(), this->scored_phrases.end(), PhraseCompare());
  return;
}
