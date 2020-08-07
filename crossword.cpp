#include <vector>
#include <set>
#include <string>
#include <utility>
#include <iostream>
#include <algorithm>
#include <limits>

class WordWithDirection {
public:
	enum class Direction {
		HORIZONTAL,
		VERTICAL
	};
	WordWithDirection (const char* text,  Direction direction)
	: _text (text), _direction(direction) {
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
		    return _xStart + _text.length();
		} else {
			return _xStart + 1;
		}
	}
	int yEnd() const {
		if (_direction == Direction::HORIZONTAL) {
		    return _yStart + 1;
		} else {
			return _yStart + _text.length();
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
	: std::vector<Crossword>(puzzle)
	, _independent(puzzle._independent) {
	}
	CrosswordPuzzle(std::initializer_list<Crossword> il)
	: std::vector<Crossword>(il) {
	}
	CrosswordPuzzle(const CrosswordPuzzle& puzzle,
			WordWithDirection independent)
	: std::vector<Crossword>(puzzle) {
		_independent.insert(independent);
	}
	const std::set<WordWithDirection>& independent() const {
		return _independent;
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
			for (int x=xStart(); x<yEnd(); x++) {
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

private:
	std::set<WordWithDirection> _independent;
};

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
std::vector<CrosswordPuzzle> findCrosswordPuzzlesByBruteForce(
		const std::vector<std::string>& words,
		size_t minCrosses, size_t maxMatches) {
	std::vector<CrosswordPuzzle> result;
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
						result.push_back(puzzle);
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
void processNextCrosswordPuzzles(const CrosswordPuzzle& puzzle,
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
				pnpFunctor(puzzle, puzzleExt, wwd);
			}
		}
	}
}

class PushBackVerticalWord {
public:
	void operator()(CrosswordPuzzle& puzzle, const WordWithDirection& wwd,
			int x, int y, size_t i) {
		puzzle.emplace_back(wwd, x, y - i);
	}
};

class PushBackHorizontalWord {
public:
	void operator()(CrosswordPuzzle& puzzle, const WordWithDirection& wwd,
			int x, int y, size_t i) {
		puzzle.emplace_back(wwd, x - i, y);
	}
};

template <class PROCESS_NEXT_PUZZLE>
void processNextCrosswordPuzzles(const std::vector<CrosswordPuzzle>& puzzles,
		const WordWithDirection& wwd, PROCESS_NEXT_PUZZLE& pnpFunctor) {
	for (const CrosswordPuzzle& puzzle : puzzles) {
		for (const Crossword& cw : puzzle) {
			if (cw.direction() == wwd.direction()) {
				continue;
			}
			if (cw.direction() == Crossword::Direction::HORIZONTAL) {
				int y = cw.yStart();
				for (int x=cw.xStart(); x<cw.xEnd(); x++) {
					CrosswordPuzzle puzzleExt = puzzle;
					processNextCrosswordPuzzles<PushBackVerticalWord,
						PROCESS_NEXT_PUZZLE>(puzzleExt, x, y, wwd,
								pnpFunctor);
				}
			} else {
				int x = cw.xStart();
				for (int y=cw.yStart(); y<cw.yEnd(); y++) {
					CrosswordPuzzle puzzleExt = puzzle;
					processNextCrosswordPuzzles<PushBackHorizontalWord,
						PROCESS_NEXT_PUZZLE>(puzzleExt, x, y, wwd,
								pnpFunctor);
				}
			}
		}
	}
	if (puzzles.size() == 0) {
		CrosswordPuzzle puzzle;
		puzzle.emplace_back(wwd, 0, 0);
		pnpFunctor(CrosswordPuzzle(), puzzle, wwd);
	}
}

class AlwaysPushBack {
public:
	AlwaysPushBack(std::vector<CrosswordPuzzle>& found)
    : _found (found) {
	}
	void operator()(const CrosswordPuzzle& puzzleOrigin,
			const CrosswordPuzzle& puzzleNew,
			const WordWithDirection& currentWord){
		if (puzzleNew.valid()) {
			_found.push_back(puzzleNew);
		} else {
			_found.push_back(CrosswordPuzzle(puzzleOrigin, currentWord));
		}
	}
private:
	std::vector<CrosswordPuzzle>& _found;
};

class IfValidPushBack {
public:
	IfValidPushBack(std::vector<CrosswordPuzzle>& found)
    : _found (found) {
	}
	void operator()(const CrosswordPuzzle& puzzleOrigin,
			const CrosswordPuzzle& puzzleNew,
			const WordWithDirection& currentWord){
		if (puzzleNew.valid()) {
			_found.push_back(puzzleNew);
		}
	}
private:
	std::vector<CrosswordPuzzle>& _found;
};

template <class CrosswordProgress>
std::vector<CrosswordPuzzle> findCrosswordPuzzlesBySica1(
		const std::vector<std::string>& words,
		size_t minCrosses, size_t maxMatches) {
	std::vector<CrosswordPuzzle> found;
	CrosswordProgress cp;
	std::vector<std::string> permutedWords = words;
	size_t n = 0;

	do {
		std::vector<size_t> directions (words.size(), 0);
		do {
			using D = Crossword::Direction;
			std::vector<WordWithDirection> wordsWithDirection;
			for (size_t i=0; i<permutedWords.size(); i++) {
				wordsWithDirection.emplace_back(permutedWords[i].c_str(),
						directions[i] == 0 ? D::HORIZONTAL : D::VERTICAL);
			}
			std::vector<CrosswordPuzzle> foundUnfiltered;
			for (const WordWithDirection& wwd : wordsWithDirection) {
				std::vector<CrosswordPuzzle> nextFounds;
				AlwaysPushBack alwaysPushBack(nextFounds);
				processNextCrosswordPuzzles<AlwaysPushBack>(foundUnfiltered,
						wwd, alwaysPushBack);
				foundUnfiltered.insert(foundUnfiltered.begin(),
						nextFounds.begin(), nextFounds.end());
			}
//			std::vector<CrosswordPuzzle> foundUnfilteredExt;
//			IfValidPushBack ifValidPushBack(foundUnfilteredExt);
//			for (const CrosswordPuzzle& puzzle : foundUnfiltered) {
//				std::vector<CrosswordPuzzle> nextFounds;
//				IfValidPushBack ifValidPushBack(nextFounds);
//				for (const WordWithDirection& iw: puzzle.independent()) {
//					processNextCrosswordPuzzles(foundUnfiltered, iw,
//							ifValidPushBack);
//				}
//				foundUnfilteredExt.insert(foundUnfilteredExt.begin(),
//						nextFounds.begin(), nextFounds.end());
//			}
//			foundUnfiltered.insert(foundUnfiltered.begin(),
//					foundUnfilteredExt.begin(), foundUnfilteredExt.end());
			for (const CrosswordPuzzle& foundPuzzle : foundUnfiltered) {
				size_t c = foundPuzzle.crosses();
				if (c >= minCrosses) {
					found.push_back(foundPuzzle);
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

struct CrosswordProgressPrinter {
	void foundSolution(const CrosswordPuzzle& puzzle, size_t crosses,
			size_t iterations) {
		std::cout << "Found solution (";
		std::cout << crosses << " crosses, iterations=" << iterations << "):\n";
		std::cout << "===============\n";
		std::cout << puzzle.toString() << '\n';
		std::cout << "===============\n";
	}
	void nextIteration(size_t n) {
		if (n % 10000000 == 0 && n != 0) {
			std::cout << "Searched " << n / 1000000;
			std::cout << " million variants.\n";
		}
	}
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
	std::vector<std::string> words = {"MAIWANDERUNG", "NEUN", "SONNE", "RADWEG",
			"BAZAR"};
	std::vector<CrosswordPuzzle> foundCrosswords =
			findCrosswordPuzzlesBySica1<CrosswordProgressPrinter>(
					words, 4, 100000);
	std::cout << "Found " << foundCrosswords.size() << " matching puzzles.\n";
}
