#include <vector>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <iostream>
#include <algorithm>
#include <limits>
#include <chrono>

class WordWithDirection {
public:
	enum class Direction {
		HORIZONTAL,
		VERTICAL
	};
	WordWithDirection (const char* text,  Direction direction)
	: _text (text), _direction(direction) {
	}
	std::string text() const {
		return _text;
	}
	char operator[](size_t i) const {
		return _text[i];
	}
	bool operator<(const WordWithDirection& other) const {
		if (_direction != other._direction) {
			return _direction == Direction::HORIZONTAL ? true : false;
		} else {
			return _text < other._text;
		}
	}
	size_t length() const {
		return _text.length();
	}
	Direction direction() const {
		return _direction;
	}
protected:
	std::string _text;
	Direction _direction;
};

class Crossword : public WordWithDirection{
public:
	Crossword (const char* text, int xStart, int yStart, Direction direction)
	: WordWithDirection (text, direction),
	  _xStart(xStart), _yStart(yStart){
	}
	Crossword (const WordWithDirection& wwd, int xStart, int yStart)
	: WordWithDirection (wwd), _xStart(xStart), _yStart(yStart){
	}
	int xStart() const {
		return _xStart;
	}
	int yStart() const {
		return _yStart;
	}
	bool operator<(const Crossword& other) const {
		if (_direction != other._direction) {
			return _direction == Direction::HORIZONTAL;
		}
		if (_yStart != other._yStart) {
			return _yStart < other._yStart;
		}
		if (_xStart != other._xStart) {
			return _xStart < other._xStart;
		}
		return false;
	}
	char character(int x, int y) const {
		int textLength = static_cast<int>(_text.length());
		if (_direction == Direction::HORIZONTAL && _yStart == y) {
			if (x >= _xStart && x <= _xStart + textLength) {
				return _text[x - _xStart];
			}
		} else if (_direction == Direction::VERTICAL && _xStart == x) {
			if (y >= _yStart && y <= _yStart + textLength) {
				return _text[y - _yStart];
			}
		}
		return 0;
	}
	int xEnd() const {
		if (_direction == Direction::HORIZONTAL) {
		    return _xStart + static_cast<int>(_text.length());
		} else {
			return _xStart + 1;
		}
	}
	int yEnd() const {
		if (_direction == Direction::HORIZONTAL) {
		    return _yStart + 1;
		} else {
			return _yStart + static_cast<int>(_text.length());
		}
	}
private:
	int _xStart;
	int _yStart;
};

class CrosswordPuzzle : public std::vector<Crossword> {
public:
	CrosswordPuzzle()
	: std::vector<Crossword>() {
	}
	CrosswordPuzzle(const CrosswordPuzzle& puzzle)
	: std::vector<Crossword>(puzzle) {
	}
	CrosswordPuzzle(std::initializer_list<Crossword> il)
	: std::vector<Crossword>(il) {
	}
	int xStart() const {
		int xStart = std::numeric_limits<int>::max();
		for (const Crossword& word : *this) {
			xStart = std::min(xStart, word.xStart());
		}
		return xStart;
	}
	int xEnd() const {
		int xEnd = std::numeric_limits<int>::min();
		for (const Crossword& word : *this) {
			xEnd = std::max(xEnd, word.xEnd());
		}
		return xEnd;
	}
	int yStart() const {
		int yStart = std::numeric_limits<int>::max();
		for (const Crossword& word : *this) {
			yStart = std::min(yStart, word.yStart());
		}
		return yStart;
	}
	int yEnd() const {
		int yEnd = std::numeric_limits<int>::min();
		for (const Crossword& word : *this) {
			yEnd = std::max(yEnd, word.yEnd());
		}
		return yEnd;
	}
	std::vector<std::pair<char, const Crossword*>> characters(int x,
			int y) const {
		std::vector<std::pair<char, const Crossword*>> founds;
		for (size_t i=0; i<size(); i++) {
			const Crossword& word = operator[](i);
			char c = word.character(x, y);
			if (c) {
				founds.push_back({c, &word});
			}
		}
		return founds;
	}

	bool valid() const {
		using CharactersInWord = std::vector<std::pair<char, const Crossword*>>;

	    struct HorizontalCharacters {
	    	int lineIndex;
	    	int rowIndex;
	    	CharactersInWord operator()(const CrosswordPuzzle& puzzle) const {
	    		return puzzle.characters(lineIndex, rowIndex);
	    	}
	    };
	    struct VerticalCharacters {
	    	int lineIndex;
	    	int rowIndex;
	    	CharactersInWord operator()(const CrosswordPuzzle& puzzle) const {
	    		return puzzle.characters(rowIndex, lineIndex);
	    	}
	    };

	    if (validImpl<HorizontalCharacters>(xStart(), xEnd(), yStart(), yEnd())) {
		    if (validImpl<VerticalCharacters>(yStart(), yEnd(), xStart(), xEnd())) {
	            return true;
	        }
	    }
	    return false;
	}
	size_t crosses() const {
		using CharacterInWord = std::pair<char, const Crossword*>;
	    using CharactersInWord = std::vector<CharacterInWord>;
	    size_t result = 0;

		for (int y=yStart(); y<yEnd(); y++) {
			for (int x=xStart(); x<xEnd(); x++) {
				CharactersInWord csiw = characters(x, y);
				if (csiw.size() > 1) {
					result++;
				}
			}
		}
		return result;
	}
	std::string toString() const {
		using CharacterInWord = std::pair<char, const Crossword*>;
	    using CharactersInWord = std::vector<CharacterInWord>;
	    const size_t w = xEnd() - xStart();
	    const size_t h = yEnd() - yStart();
	    std::string result((w + 1) * h, '\n');
	    size_t i = 0;

		for (int y=yStart(); y<yEnd(); y++) {
			for (int x=xStart(); x<xEnd(); x++) {
				CharactersInWord csiw = characters(x, y);
				char c = ' ';
				for (const CharacterInWord& ciw : csiw) {
					if (c == ' ') {
					    c = ciw.first;
					} else if (c != ciw.first) {
						c = '*';
					}
				}
				result[i] = c;
				i++;
			}
			i++;
		}
		return result;
	}

protected:
	template<class CHARACTERS>
	bool validImpl(int rowStart, int rowEnd, int lineStart, int lineEnd) const {
		using CharactersInWord = std::vector<std::pair<char, const Crossword*>>;
		const Crossword* owner = nullptr;
		const Crossword* partner = nullptr;
		bool ownerPartnerClarified = false;

		for (int lineIndex=lineStart; lineIndex < lineEnd; lineIndex++) {
			for (int rowIndex=rowStart; rowIndex < rowEnd; rowIndex++) {
				CharactersInWord ciw = CHARACTERS{rowIndex, lineIndex}(*this);
				if (ciw.size() > 2) {
					return false;
				}
				if (owner != nullptr && partner != nullptr) {
	                if (ciw.size() == 2) {
					    return false;
	                } else if (ciw.size() == 1) {
	                	if(ownerPartnerClarified && ciw[0].second != owner) {
	                		return false;
	                	} else if(!ownerPartnerClarified) {
	                		if (ciw[0].second == partner) {
	                    		owner = partner;
	                		} else if (ciw[0].second != owner){
	                			return false;
	                		}
	                		ownerPartnerClarified = true;
	                    }
					} else {
						owner = nullptr;
					}
					partner = nullptr;
				} else if (owner != nullptr) {
					if (ciw.size() == 2) {
						if (ciw[0].first != ciw[1].first) {
							return false;
						}
						if (ciw[0].second == owner) {
							partner = ciw[1].second;
						} else if (ciw[1].second == owner) {
							partner = ciw[0].second;
						} else {
							return false;
						}
					} else if (ciw.size() == 1) {
						if (ciw[0].second != owner) {
							return false;
						}
					} else {
						owner = nullptr;
					}
				} else {
					if (ciw.size() == 2) {
						if (ciw[0].first != ciw[1].first) {
							return false;
						}
						owner = ciw[0].second;
						partner = ciw[1].second;
						ownerPartnerClarified = false;
					} else if (ciw.size() == 1) {
						owner = ciw[0].second;
						ownerPartnerClarified = true;
					}
				}
			}
			owner = nullptr;
			partner = nullptr;
		}
		return true;
	}
};

CrosswordPuzzle normalizedPuzzle (const CrosswordPuzzle& puzzle) {
	int xPuzzleStart = puzzle.xStart();
	int yPuzzleStart = puzzle.yStart();
	CrosswordPuzzle result;
	for (Crossword cw : puzzle) {
		result.emplace_back(cw, cw.xStart() - xPuzzleStart,
				cw.yStart() - yPuzzleStart);
	}
	return result;
}

class TestFailed : public std::exception {
public:
	TestFailed(const std::string& message)
	: _message (message) {
	}
    virtual const char* what() const noexcept {
    	return _message.c_str();
    }
private:
	std::string _message;
};

void assertTrue(const std::string& checkDescription, bool expression) {
	if (!expression) {
		throw TestFailed("Assertion failed: " + checkDescription);
	}
}

void test_toString() {
	using Direction = Crossword::Direction;
	CrosswordPuzzle crossword = {
			{"MAIWANDERUNG", 0, 4, Direction::HORIZONTAL},
			{"NEUN", 10, 4, Direction::VERTICAL},
			{"SONNE", 5, 2, Direction::VERTICAL},
			{"RADWEG", 1, 6, Direction::HORIZONTAL},
			{"BAZAR", 8, 0, Direction::VERTICAL},
	};
	std::string expectedString =
			"        B   \n"
			"        A   \n"
			"     S  Z   \n"
			"     O  A   \n"
			"MAIWANDERUNG\n"
			"     N    E \n"
			" RADWEG   U \n"
			"          N \n";
	assertTrue ("toString() works as expected",
			expectedString == crossword.toString());
}

void test_valid() {
	using Direction = Crossword::Direction;
	CrosswordPuzzle puzzle1 = {
			{"MAIWANDERUNG", 0, 4, Direction::HORIZONTAL},
			{"NEUN", 10, 4, Direction::VERTICAL},
			{"SONNE", 5, 2, Direction::VERTICAL},
			{"RADWEG", 1, 6, Direction::HORIZONTAL},
			{"BAZAR", 8, 0, Direction::VERTICAL},
	};
	CrosswordPuzzle puzzle2 = {
			{"MAIWANDERUNG", 0, 4, Direction::HORIZONTAL},
			{"NEUN", 5, 5, Direction::VERTICAL},
			{"SONNE", 5, 2, Direction::VERTICAL},
			{"RADWEG", 1, 6, Direction::HORIZONTAL},
			{"BAZAR", 8, 0, Direction::VERTICAL},
	};
	CrosswordPuzzle puzzle3 = {
			{"MAIWANDERUNG", 0, 4, Direction::HORIZONTAL},
			{"NEUN", 5, 5, Direction::HORIZONTAL},
			{"SONNE", 5, 2, Direction::VERTICAL},
			{"RADWEG", 1, 6, Direction::HORIZONTAL},
			{"BAZAR", 8, 0, Direction::VERTICAL},
	};
	CrosswordPuzzle puzzle4 = {
			{"MAIWANDERUNG", 0, 4, Direction::HORIZONTAL},
			{"NEUN", 10, 4, Direction::VERTICAL},
			{"SONNE", 5, 1, Direction::VERTICAL},
			{"RADWEG", 1, 6, Direction::HORIZONTAL},
			{"BAZAR", 8, 0, Direction::VERTICAL},
	};
	CrosswordPuzzle puzzle5 = {
			{"MAIWANDERUNG", 0, 0, Direction::HORIZONTAL},
			{"NEUN", 0, 2, Direction::HORIZONTAL},
	};
	CrosswordPuzzle puzzle6 = {
			{"MAIWANDERUNG", 0, 0, Direction::VERTICAL},
			{"NEUN", 0, 5, Direction::HORIZONTAL},
	};
	CrosswordPuzzle puzzle7 = {
			{"MAIWANDERUNG", 0, 0, Direction::VERTICAL},
			{"RADWEG", 0, 1, Direction::HORIZONTAL},
	};

	assertTrue("crossword1 is valid", puzzle1.valid());
	assertTrue("crossword2 is not valid", !puzzle2.valid());
	assertTrue("crossword3 is not valid", !puzzle3.valid());
	assertTrue("crossword4 is not valid", !puzzle4.valid());
	assertTrue("crossword5 is valid", puzzle5.valid());
	assertTrue("crossword6 is valid", puzzle6.valid());
	assertTrue("crossword7 is not valid", !puzzle7.valid());
}

template<typename T>
bool increaseByOne (std::vector<T>& v,
		size_t maxElementValue) {
	bool carryFlag = false;

	for (size_t& value : v) {
		if (carryFlag) {
			if (value != maxElementValue) {
				value = value + 1;
				carryFlag = false;
				break;
			} else {
				value = 0;
			}
		} else if (value == maxElementValue) {
			value = 0;
			carryFlag = true;
		} else {
			value = value + 1;
			break;
		}
	}
	return !carryFlag;
}

template <class CrosswordProgress>
std::set<CrosswordPuzzle> findCrosswordPuzzlesByBruteForce(
		const std::vector<std::string>& words,
		size_t minCrosses, size_t maxMatches) {
	std::set<CrosswordPuzzle> result;
	std::vector<std::string>::const_iterator itMaxString =
			std::max_element(words.begin(), words.end(),
			[](const std::string& a, const std::string& b){
				return a.length() < b.length();});
	const size_t maxLength = itMaxString->length();
	std::vector<int> yValues (words.size(), 0);
	size_t n = 0;
	CrosswordProgress cp;

	do {
		std::vector<int> xValues (words.size(), 0);
		do {
			std::vector<int> directions (words.size(), 0);
			do {
				CrosswordPuzzle puzzle;
				for (size_t i=0; i<words.size(); i++) {
					using D = Crossword::Direction;
					puzzle.emplace_back(words[i].c_str(),
							xValues[i], yValues[i],
							directions[i] == 0 ? D::HORIZONTAL : D::VERTICAL);
				}
				if (puzzle.valid()) {
					size_t c = puzzle.crosses();
					if (c >= minCrosses) {
						cp.foundSolution(puzzle, c, n);
						result.insert(puzzle);
						if (result.size() >= maxMatches) {
							return result;
						}
					}
				}
				cp.nextIteration(n);
				n++;
			} while (increaseByOne(directions, 1));
		} while (increaseByOne(xValues, maxLength));
	} while (increaseByOne(yValues, maxLength));
	return result;
}

template <class PUSH_BACK_TO_PUZZLE, class PROCESS_NEXT_PUZZLE>
bool processNextCrosswordPuzzles(const CrosswordPuzzle& puzzle,
		int x, int y, const WordWithDirection& wwd,
		PROCESS_NEXT_PUZZLE& pnpFunctor) {
	using CharactersInWord = std::vector<std::pair<char, const Crossword*>>;
	CharactersInWord ciw = puzzle.characters(x, y);
	char currentChar = ciw[0].first;
	if (ciw.size() == 1) {
		PUSH_BACK_TO_PUZZLE pbtp;
		for (size_t i=0; i<wwd.length(); i++) {
			if (currentChar == wwd[i]) {
				CrosswordPuzzle puzzleExt = puzzle;
				pbtp(puzzleExt, wwd, x, y, i);
				if (pnpFunctor(puzzle, puzzleExt, wwd)) {
					return true;
				}
			}
		}
	}
	return false;
}

class PushBackVerticalWord {
public:
	void operator()(CrosswordPuzzle& puzzle, const WordWithDirection& wwd,
			int x, int y, size_t i) {
		puzzle.emplace_back(wwd, x, y - static_cast<int>(i));
	}
};

class PushBackHorizontalWord {
public:
	void operator()(CrosswordPuzzle& puzzle, const WordWithDirection& wwd,
			int x, int y, size_t i) {
		puzzle.emplace_back(wwd, x - static_cast<int>(i), y);
	}
};

class StoreValidPuzzle {
public:
	StoreValidPuzzle(std::vector<CrosswordPuzzle>& found)
    : _found (found) {
	}
	bool operator()(const CrosswordPuzzle& puzzleOrigin,
			const CrosswordPuzzle& puzzleNew,
			const WordWithDirection& currentWord){
		if (puzzleNew.valid()) {
			_found.push_back(puzzleNew);
			return true;
		}
		return false;
	}
private:
	std::vector<CrosswordPuzzle>& _found;
};

std::vector<CrosswordPuzzle> findCrosswordPuzzles(
		const std::vector<CrosswordPuzzle>& puzzles,
		const WordWithDirection& wwd) {
	std::vector<CrosswordPuzzle> result;
	StoreValidPuzzle storeValidPuzzle(result);
	for (const CrosswordPuzzle& puzzle : puzzles) {
		for (const Crossword& cw : puzzle) {
			if (cw.direction() == wwd.direction()) {
				continue;
			}
			if (cw.direction() == Crossword::Direction::HORIZONTAL) {
				int y = cw.yStart();
				for (int x=cw.xStart(); x<cw.xEnd(); x++) {
					CrosswordPuzzle puzzleExt = puzzle;
					if (processNextCrosswordPuzzles<PushBackVerticalWord>(
							puzzleExt, x, y, wwd, storeValidPuzzle)) {
						break;
					}
				}
			} else {
				int x = cw.xStart();
				for (int y=cw.yStart(); y<cw.yEnd(); y++) {
					CrosswordPuzzle puzzleExt = puzzle;
					if (processNextCrosswordPuzzles<PushBackHorizontalWord>(
							puzzleExt, x, y, wwd, storeValidPuzzle)) {
						break;
					}
				}
			}
		}
	}
	if (puzzles.size() == 0) {
		CrosswordPuzzle puzzle;
		puzzle.emplace_back(wwd, 0, 0);
		result.push_back(puzzle);
	}
	return result;
}

size_t factorial(size_t n) {
	size_t result = 1;
    for(size_t i=1; i<=n; ++i)
    {
        result *= i;
    }
    return result;
}

size_t power(size_t x, size_t p) {
  if (p == 0) return 1;
  if (p == 1) return x;
  return x * power(x, p-1);
}

template <class CrosswordProgress>
std::set<CrosswordPuzzle> findCrosswordPuzzlesBySica1(
		const std::vector<std::string>& words,
		size_t minCrosses, size_t maxMatches) {
	std::set<CrosswordPuzzle> found;
	size_t n = 0;
	std::vector<std::string> permutedWords = words;
	CrosswordProgress cp(factorial(words.size()) *
			(power(2, words.size() / 2)));
	// Sort words to get all permutations.
	std::sort(permutedWords.begin(), permutedWords.end());

	do {
		std::vector<size_t> directions (words.size(), 0);
		for (size_t i=0; i<words.size(); i++) {
			directions[i] = i%2;
		}
		do {
			using D = Crossword::Direction;
			std::vector<WordWithDirection> wordsWithDirection;
			for (size_t i=0; i<permutedWords.size(); i++) {
				wordsWithDirection.emplace_back(permutedWords[i].c_str(),
						directions[i] == 0 ? D::HORIZONTAL : D::VERTICAL);
			}
			std::vector<CrosswordPuzzle> foundUnfiltered;
			for (const WordWithDirection& wwd : wordsWithDirection) {
				foundUnfiltered = findCrosswordPuzzles(foundUnfiltered, wwd);
			}
			for (const CrosswordPuzzle& foundPuzzle : foundUnfiltered) {
				size_t c = foundPuzzle.crosses();
				if (foundPuzzle.size() == words.size() && c >= minCrosses) {
					CrosswordPuzzle np = normalizedPuzzle(foundPuzzle);
					if (found.find(np) != found.end()) {
						continue;
					}
					found.insert(np);
					cp.foundSolution(foundPuzzle, c, n);
					if (found.size() >= maxMatches) {
						return found;
					}
				}
			}
			cp.nextIteration(n);
			n++;
		} while (increaseByOne(directions, 1));
	} while (std::next_permutation(permutedWords.begin(), permutedWords.end()));
	return found;
}

class EmplaceStringVertical {
public:
	void operator()(CrosswordPuzzle& puzzle, const std::string& word,
			int x, int y, size_t i) {
		puzzle.emplace_back(WordWithDirection(word.c_str(),
				WordWithDirection::Direction::VERTICAL), x, y - static_cast<int>(i));
	}
};

class EmplaceStringHorizontal {
public:
	void operator()(CrosswordPuzzle& puzzle, const std::string& word,
			int x, int y, size_t i) {
		puzzle.emplace_back(WordWithDirection(word.c_str(),
				WordWithDirection::Direction::HORIZONTAL), x - static_cast<int>(i), y);
	}
};

template <class EMPLACE_STRING_TO_PUZZLE, class PROGRESS_TRACER>
CrosswordPuzzle findAnyPuzzle(const CrosswordPuzzle& puzzle,
		int x, int y, const std::string& word, PROGRESS_TRACER& pt) {
	using CharactersInWord = std::vector<std::pair<char, const Crossword*>>;
	CrosswordPuzzle result;
	CharactersInWord ciw = puzzle.characters(x, y);
	char currentChar = ciw[0].first;
	if (ciw.size() == 1) {
		EMPLACE_STRING_TO_PUZZLE estp;
		for (size_t i=0; i<word.length(); i++) {
			if (currentChar == word[i]) {
				CrosswordPuzzle puzzleExt = puzzle;
				estp(puzzleExt, word, x, y, i);
				pt.validCheck(puzzle, puzzleExt, word);
				if (puzzleExt.valid()) {
					result = puzzleExt;
					break;
				}
			}
		}
	}
	return result;
}

template<class EMPLACE_STRING_TO_PUZZLE, class PROGRESS_TRACER>
std::vector<CrosswordPuzzle> findPuzzles(
		const CrosswordPuzzle& puzzle, const std::string& word,
		int x, int y, const std::vector<std::string>& remainingWords,
		size_t minCrosses, size_t minPuzzles, PROGRESS_TRACER& pt) {
	std::vector<CrosswordPuzzle> result;
	CrosswordPuzzle puzzleExt = findAnyPuzzle<EMPLACE_STRING_TO_PUZZLE>(puzzle,
			x, y, word, pt);
	if (puzzleExt.size() > 0) {
		std::vector<CrosswordPuzzle> found = findPuzzles(
				puzzleExt, remainingWords, minCrosses,
				minPuzzles, pt);
		std::copy_if (found.begin(), found.end(),
				std::back_inserter(result),
				[minCrosses](const CrosswordPuzzle& p){
			return p.crosses() >= minCrosses;
		});
	}
	return result;
}

template<class PROGRESS_TRACER>
std::vector<CrosswordPuzzle> findPuzzles(const CrosswordPuzzle& puzzle,
		const std::vector<std::string>& words, size_t minCrosses,
		size_t minPuzzles, PROGRESS_TRACER& pt) {
	using D = WordWithDirection::Direction;
	std::vector<CrosswordPuzzle> result;

	if (words.size() == 0) {
		result.push_back(puzzle);
		return result;
	}
	if (minPuzzles == 0) {
		return result;
	}
	for (const std::string& word : words) {
		std::vector<std::string> remainingWords(words);
		auto itWord = std::find(remainingWords.begin(),
				remainingWords.end(), word);
		remainingWords.erase(itWord);
		for (const Crossword& cw : puzzle) {
			if (cw.direction() == D::HORIZONTAL) {
				int y = cw.yStart();
				for (int x=cw.xStart(); x<cw.xEnd(); x++) {
					std::vector<CrosswordPuzzle> found =
							findPuzzles<EmplaceStringVertical>(puzzle, word,
									x, y, remainingWords,
									minCrosses, minPuzzles, pt);
					std::copy(found.begin(), found.end(),
							std::back_inserter(result));
					if (result.size() >= minPuzzles) {
						return result;
					}
				}
			} else {
				int x = cw.xStart();
				for (int y=cw.yStart(); y<cw.yEnd(); y++) {
					std::vector<CrosswordPuzzle> found =
							findPuzzles<EmplaceStringHorizontal>(puzzle, word,
									x, y, remainingWords,
									minCrosses, minPuzzles, pt);
					std::copy(found.begin(), found.end(),
							std::back_inserter(result));
					if (result.size() >= minPuzzles) {
						return result;
					}
				}
			}
		}
	}
	return result;
}

class SimpleProgressTracer {
public:
	SimpleProgressTracer()
	: _numberOfValidChecks(0) {
	}
	void validCheck(const CrosswordPuzzle& puzzleOld,
			const CrosswordPuzzle& puzzleNew, const std::string& word) {
		_numberOfValidChecks++;
		if (_numberOfValidChecks % 100000 == 0) {
			std::cout << "Searched " << _numberOfValidChecks << " variants.\n";
		}
	}
	size_t numberOfValidChecks() const {
		return _numberOfValidChecks;
	}
private:
	size_t _numberOfValidChecks;
};

template<class PROGRESS_TRACER>
std::vector<CrosswordPuzzle> findPuzzles(const std::vector<std::string>& words,
		size_t minCrosses, size_t minPuzzles, PROGRESS_TRACER& progressTracer) {
	std::vector<CrosswordPuzzle> puzzles;
	for (const std::string& word : words) {
		std::vector<std::string> remainingWords(words);
		auto itWord = std::find(remainingWords.begin(), remainingWords.end(),
				word);
		if (itWord != remainingWords.end()) {
			remainingWords.erase(itWord);
		}
		CrosswordPuzzle puzzleStartHorizontal;
		puzzleStartHorizontal.emplace_back(word.c_str(), 0, 0,
				WordWithDirection::Direction::HORIZONTAL);
		std::vector<CrosswordPuzzle> foundHorizontal = findPuzzles(
				puzzleStartHorizontal,remainingWords, minCrosses,
				minPuzzles - puzzles.size(), progressTracer);
		puzzles.insert(puzzles.begin(),
				foundHorizontal.begin(), foundHorizontal.end());
		if (puzzles.size() >= minPuzzles) {
			break;
		}
		CrosswordPuzzle puzzleStartVertical;
		puzzleStartHorizontal.emplace_back(word.c_str(), 0, 0,
				WordWithDirection::Direction::VERTICAL);
		std::vector<CrosswordPuzzle> foundVertical = findPuzzles(
				puzzleStartVertical, remainingWords, minCrosses,
				minPuzzles - puzzles.size(), progressTracer);
		puzzles.insert(puzzles.begin(),
				foundVertical.begin(), foundVertical.end());
		if (puzzles.size() >= minPuzzles) {
			break;
		}
	}
	return puzzles;
}

class CrosswordProgressPrinter {
public:
	CrosswordProgressPrinter(size_t numberOfVariants)
    : _numberOfVariants(numberOfVariants) {
	}
	void foundSolution(const CrosswordPuzzle& puzzle, size_t crosses,
			size_t iterations) {
		std::cout << "Found solution (";
		std::cout << crosses << " crosses, iterations=" << iterations << "):\n";
		std::cout << "===============\n";
		std::cout << puzzle.toString() << '\n';
		std::cout << "===============\n";
	}
	void nextIteration(size_t n) {
		if (n % 100 == 0 && n != 0) {
			std::cout << "Searched " << n << " of " << _numberOfVariants;
			std::cout << " variants.\n";
		}
	}
private:
	size_t _numberOfVariants;
};

int main() {
	size_t numberOfFailedTests = 0;
	try {
		test_toString();
	} catch (const TestFailed& e) {
		std::cerr << e.what() << '\n';
		numberOfFailedTests++;
	}
	try {
		test_valid();
	} catch (const TestFailed& e) {
		std::cerr << e.what() << '\n';
		numberOfFailedTests++;
	}
	if (numberOfFailedTests > 0) {
		std::cerr << numberOfFailedTests << " test failed.\n";
	} else {
		std::cout << "All tests successful.\n";
	}
	auto start = std::chrono::steady_clock::now();
//	std::vector<std::string> words = {"DEHNEN", "NIKOLAUS", "NEUREUTHER",
//			"SOELDEN", "RUNDLAUF", "DREI", "HOCKE", "BUEGELEISEN", "FIS",
//	        "HUENDLE", "STELLER", "MAIWANDERUNG", "MARKUS", "ELENA", "PETRA",
//	        "XAVER", "XELSBOCK", "SYSTEM", "ROLLADEN", "BUCH"};
	std::vector<std::string> words = {"DEHNEN", "NIKOLAUS", "NEUREUTHER",
			"SOELDEN", "RUNDLAUF", "DREI", "HOCKE", "BUEGELEISEN", "FIS"};
//	std::set<CrosswordPuzzle> foundCrosswords =
//			findCrosswordPuzzlesBySica1<CrosswordProgressPrinter>(
//					words, 22, 100000);
//	std::vector<std::string> words = {"MAIWANDERUNG", "NEUN", "SONNE", "RADWEG",
//			"BAZAR"};
//	std::set<CrosswordPuzzle> foundCrosswords =
//			findCrosswordPuzzlesBySica1<CrosswordProgressPrinter>(
//					words, 4, 100000);
//	std::cout << "Found " << foundCrosswords.size() << " matching puzzles.\n";
	SimpleProgressTracer progressTracer;
	auto foundPuzzles = findPuzzles(words, 10, 1, progressTracer);
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds = end-start;
	for (CrosswordPuzzle puzzle : foundPuzzles) {
		std::cout << "Next Puzzle (" << puzzle.crosses() << " crosses):\n";
		std::cout << puzzle.toString() << "\n";
	}
	std::cout << "Found " << foundPuzzles.size() << " puzzles\n";
	std::cout << "Tried " << progressTracer.numberOfValidChecks() << " variants\n";
	std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";
}
