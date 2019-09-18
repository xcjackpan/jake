# jake

JAKE = Jack's Automatic Keyword Extraction. Modified version of RAKE.

* RAKE simply takes the sum of member word scores for the score of a phrase. This allows long phrases with irrelevant words to score highly above shorter ones. JAKE punishes long key phrases where each individual word has a low score.

* RAKE calculates each score by degree/frequency. The degree of a node doesn't tell you how popular its neighbours are despite
us having this information in the co-occurance graph. JAKE factors in the popularity of each node's neighbours.

_RAKE, short for Rapid Automatic Keyword Extraction, is a domain independent keyword extraction algorithm 
which tries to determine key phrases in a body of text by analyzing the frequency of word appearance and its co-occurance
with other words in the text._

RAKE is described in this paper: https://www.researchgate.net/publication/227988510_Automatic_Keyword_Extraction_from_Individual_Documents
