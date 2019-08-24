#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <ctype.h>
#include <cctype>
#include <algorithm>
#include <iterator>
#include <queue>


// Rake implemented as a C++ class following along:
// https://www.researchgate.net/publication/227988510_Automatic_Keyword_Extraction_from_Individual_Documents

// RAKE, short for Rapid Automatic Keyword Extraction, is a domain independent keyword extraction 
// algorithm which tries to determine key phrases in a body of text by analyzing the frequency of word appearance
// and its co-occurance with other words in the text.

class Rake {
  std::string stopwordsFileName;
  std::unordered_set<std::string> stopwords;
  std::string documentName;
  std::vector<std::string> document;
  std::vector<std::pair<std::string, int>> scored_phrases;

  static char toLower(char c) {
      if (c <= 'Z' && c >= 'A') {
          return c - ('Z' - 'z');
      }
      return c;
  }

  // Comparator for scoring phrases
  class PhraseCompare {
    public:
      bool operator() (std::pair<std::string, int> a, std::pair<std::string, int> b) {
        return a.second > b.second;
      }
  };

  public:
    std::vector<std::string> getScoredPhrases(int num) {
      std::vector<std::string> result;
      if (num > this->scored_phrases.size()) {
        return {};
      } else {
        for (int i = 0; i < num; i++) {
          result.emplace_back(this->scored_phrases[i].first);
        }
      }
      return result;
    }

    void rake() {

      // Isolate candidate phrases
      std::string phrase = "";
      std::vector<std::string> phrases;
      std::unordered_map<std::string, int> candidate_indices; // Maps a number to each candidate word, used for computing co-occurences
      int candidate_count = 0;
      for (auto it = this->document.begin(); it != this->document.end(); it++) {
        std::string word = *it;
        std::string word_lower = *it;
        std::transform(word_lower.begin(), word_lower.end(), word_lower.begin(), this->toLower);
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
          std::transform(currtoken.begin(), currtoken.end(), currtoken.begin(), this->toLower);
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
      std::vector<double> word_scores (candidate_count, 0); // word_score[i] is the score for word with index i
      for (auto it = candidate_indices.begin(); it != candidate_indices.end(); it++) {
        // freq(w) is matrix[w_index][w_index]
        // deg(w) is sum of all matrix[w_index][n] and matrix[n][w_index] minus matrix[w_index][w_index]
        int index = it->second;
        double deg = 0;
        for (int i = 0; i < candidate_count; i++) {
          for (int j = 0; j < candidate_count; j++) {
            if ((i != j) && (i == index || j == index)) {
              deg += matrix[i][j];
            }
          }
        }
        double freq = matrix[index][index];
        word_scores[index] = deg/freq; // Frequency is never 0
      }

      // Assign each phrase a score, based on the sum of its member words' scores
      this->scored_phrases = {};
      for (auto it = phrases.begin(); it != phrases.end(); it++) {
        std::istringstream iss(*it);
        std::string word = "";
        int phrase_score = 0;
        while (iss >> word) {
          std::transform(word.begin(), word.end(), word.begin(), this->toLower);
          int index = candidate_indices.find(word)->second;
          phrase_score += word_scores[index];
        }
        this->scored_phrases.emplace_back(std::make_pair(*it, phrase_score));
      }

      std::sort(this->scored_phrases.begin(), this->scored_phrases.end(), PhraseCompare());
      return;
    }

    Rake(std::string stopwordsFileName, std::string documentName) : stopwordsFileName{stopwordsFileName},
                                                                    documentName{documentName} {
      // Get stopwords
      std::string line;
      std::ifstream stopwordsFile(this->stopwordsFileName);

      if (stopwordsFile)  {
        while (std::getline(stopwordsFile, line)) {
          this->stopwords.insert(line);
        }
        stopwordsFile.close();
      }


      // Get document body
      std::string word;
      std::ifstream documentFile(this->documentName);
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
};

int main() {
  Rake r = Rake("stopwords.txt", "document.txt");
  r.rake();

  std::cout << r.getScoredPhrases(10)[0] << std::endl;
}