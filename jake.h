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
  std::string stopwordsFileName;
  std::unordered_set<std::string> stopwords;
  std::string documentName;
  std::vector<std::string> document;
  std::vector<std::pair<std::string, double>> scoredPhrases;
  static char toLower(char c);
  class PhraseCompare;

  public:
    Jake(std::string stopwordsFileName, std::string documentName);
    std::vector<std::string> getScoredPhrases(int num);
    void processText();
};