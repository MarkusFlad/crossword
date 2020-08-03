#include <vector>
#include <string>
#include <utility>
#include <iostream>

class Crossword {
public:
	enum class Direction {
		HORIZONTAL,
		VERTICAL
	};
	Crossword (const char* text, size_t xStart, size_t yStart, Direction direction)
	: _text (text), _xStart(xStart), _yStart(yStart), _direction(direction) {
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
	char character(size_t x, size_t y) const {
		if (_direction == Direction::HORIZONTAL && _yStart == y) {
			if (x >= _xStart && x <= _xStart + _text.length()) {
				return _text[x - _xStart];
			}
		} else if (_direction == Direction::VERTICAL && _xStart == x) {
			if (y >= _yStart && y <= _yStart + _text.length()) {
				return _text[y - _yStart];
			}
		}
		return 0;
	}
	size_t xEnd() const {
		if (_direction == Direction::HORIZONTAL) {
		    return _xStart + _text.length();
		} else {
			return _xStart;
		}
	}
	size_t yEnd() const {
		if (_direction == Direction::HORIZONTAL) {
		    return _yStart;
		} else {
			return _yStart + _text.length();
		}
	}
private:
	std::string _text;
	size_t _xStart;
	size_t _yStart;
	Direction _direction;
};

size_t width(const std::vector<Crossword>& words) {
	size_t xEnd = 0;
	for (Crossword word : words) {
		xEnd = std::max(xEnd, word.xEnd());
	}
	return xEnd;
}

size_t height(const std::vector<Crossword>& words) {
	size_t yEnd = 0;
	for (Crossword word : words) {
		yEnd = std::max(yEnd, word.yEnd());
	}
	return yEnd;
}

std::vector<std::pair<char, const Crossword*>> characters(
		const std::vector<Crossword>& words, size_t x, size_t y) {
	std::vector<std::pair<char, const Crossword*>> founds;
	for (size_t i=0; i<words.size(); i++) {
		const Crossword& word = words[i];
		char c = word.character(x, y);
		if (c) {
			founds.push_back({c, &word});
		}
	}
	return founds;
}

bool crosswordValid(const std::vector<Crossword>& words) {
	using CharactersInWord = std::vector<std::pair<char, const Crossword*>>;
    const size_t w = width(words);
    const size_t h = height(words);
	const Crossword* owner = nullptr;
	const Crossword* partner = nullptr;
	bool ownerPartnerClarified = false;

	for (size_t y=0; y<h; y++) {
		for (size_t x=0; x<w; x++) {
			CharactersInWord ciw = characters(words, x, y);
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
	}
	return true;
}

std::string toString(const std::vector<Crossword>& words) {
	using CharacterInWord = std::pair<char, const Crossword*>;
    using CharactersInWord = std::vector<CharacterInWord>;
    const size_t w = width(words);
    const size_t h = height(words);
    std::string result((w + 1) * h, '\n');
    size_t i = 0;

	for (size_t y=0; y<h; y++) {
		for (size_t x=0; x<w; x++) {
			CharactersInWord csiw = characters(words, x, y);
			char c = ' ';
			for (CharacterInWord ciw : csiw) {
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

int main() {
	using WordVector = std::vector<Crossword>;
	using Direction = Crossword::Direction;

	WordVector words = {
			{"MAIWANDERUNG", 0, 4, Direction::HORIZONTAL},
			{"NEUN", 10, 4, Direction::VERTICAL},
			//{"NEUN", 5, 5, Direction::VERTICAL},
			//{"NEUN", 5, 5, Direction::HORIZONTAL},
			{"SONNE", 5, 2, Direction::VERTICAL},
			{"RADWEG", 1, 6, Direction::HORIZONTAL},
			{"BAZAR", 8, 0, Direction::VERTICAL},
	};

	std::cout << toString(words);
	std::cout << "\nCrossword is ";
	if (crosswordValid(words)) {
	    std::cout << "valid\n";
	} else {
	    std::cout << "not valid\n";
	}
}
