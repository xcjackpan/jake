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

class Jake {
  std::string stopwords_file_name;
  std::unordered_set<std::string> stopwords;
  std::string document_name;
  std::vector<std::string> document;
  std::vector<std::pair<std::string, double>> scored_phrases;
  static char to_lower(char c);
  class PhraseCompare;

  public:
    Jake(std::string stopwords_file_name, std::string document_name);
    std::vector<std::string> get_scored_phrases(unsigned num);
    void process_text();
};