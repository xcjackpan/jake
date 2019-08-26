#include "jake.h"

int main() {
  Jake j = Jake("stopwords.txt", "document.txt");
  j.processText();

  auto res = j.getScoredPhrases(10);
  for (int i = 0; i < 10; i++) {
    std::cout << res[i] << std::endl;
  }
}