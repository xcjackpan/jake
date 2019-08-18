#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <ctype.h>
#include <cctype>
#include <algorithm>

char toLower(char c) {
    if (c <= 'Z' && c >= 'A')
        return c - ('Z' - 'z');
    return c;
}

// C++ Rake implemented following along:
// https://www.researchgate.net/publication/227988510_Automatic_Keyword_Extraction_from_Individual_Documents

std::vector<std::string> rake(std::string stopwordsFileName, std::string documentName) {
  // Get stopwords
  std::string line;
  std::ifstream stopwordsFile(stopwordsFileName);
  std::unordered_set<std::string> stopwords;
  if (stopwordsFile)  {
    while (std::getline(stopwordsFile, line)) {
      stopwords.insert(line);
    }
    stopwordsFile.close();
  }

  // Get document body
  std::string word;
  std::ifstream documentFile(documentName);
  std::vector<std::string> document;
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
    document.emplace_back(word);
  }

  // Isolate phrases
  std::string phrase = "";
  std::vector<std::string> phrases;
  std::unordered_map<std::string, int> candidate_indices; // Maps a number to each candidate word, used for computing co-occurences
  for (auto it = document.begin(); it != document.end(); it++) {
    std::string word = *it;
    std::string word_lower = *it;
    std::transform(word_lower.begin(), word_lower.end(), word_lower.begin(), toLower);
    auto is_stopword = stopwords.find(word_lower);
    if (is_stopword != stopwords.end()) {
      if (phrase != "") {
        phrases.emplace_back(phrase);
      }
      phrase = "";
    } else if (word.back() == '!' || word.back() == '?' || word.back() == '.' || word.back() == ',') {
      word.pop_back();
      if (phrase != "") {
        phrase += " ";
      }
      phrase += word;
      phrases.emplace_back(phrase);
      phrase = "";
    } else {
      if (phrase != "") {
        phrase += " ";
      }
      phrase += word;
    }
  }

  for (auto it = phrases.begin(); it != phrases.end(); it++) {
    std::cout << *it << std::endl;
  }

  // Compute word co-occurences
  

  return {"a"};
}

int main() {
  std::cout << rake("stopwords.txt", "document.txt")[0] << std::endl;
}