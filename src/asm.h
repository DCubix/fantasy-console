#ifndef ASM_H
#define ASM_H

#include "console.h"

#include <string>
#include <vector>
#include <map>
#include <sstream>

enum TokenType {
	TokEnd = 0,
	TokIdentifier,
	TokNumber,
	TokReference,
	TokNewLabel,
	TokOpenBracket,
	TokCloseBracket,
	TokLet,
	TokComma
};

struct Token {
	TokenType type;
	std::string lexeme;

	Byte value;

	std::string toString() {
		std::stringstream ret;
		switch (type) {
			case TokEnd: ret << "END"; break;
			case TokIdentifier: ret << "ID(" << lexeme << ")"; break;
			case TokNumber: ret << "NUM(" << value << ")"; break;
			case TokReference: ret << "REF(&" << value << ")"; break;
			case TokNewLabel: ret << "NEWLABEL(" << lexeme << ")"; break;
			case TokOpenBracket: ret << "OPEN_BRACKET"; break;
			case TokCloseBracket: ret << "CLOSE_BRACKET"; break;
			case TokComma: ret << "COMMA"; break;
		}
		return ret.str();
	}

	Token() = default;
	Token(TokenType type, const std::string& lex, Byte val)
		: type(type), lexeme(lex), value(val)
	{}
};

class Scanner {
public:
	Scanner() = default;
	~Scanner() = default;

	Scanner(const std::string& input);

	char next();
	char current() const { return m_input[m_pos]; }
	char peek() const;
	char prev() const;
	bool hasNext() const { return m_pos < m_input.size(); }

private:
	std::string m_input;
	int m_pos;

	void advance(int n = 1);
};

using ByteList = std::vector<Byte>;
class ASM {
public:
	ASM() = default;
	~ASM() = default;

	ASM(const std::string& input, Console *console);

	void printTokens();

	void tokenize();
	ByteList compile();
private:
	void readLabelsAndRefs();

	ByteList atom();
	ByteList instruction();

	bool accept(TokenType type, const std::string& param = "", bool regex = true, bool forceCheck = false);
	bool expect(TokenType type, const std::string& param = "", bool regex = true, bool forceCheck = false);

	bool next();
	void stepBack();

	Token& current() { return m_tokens[m_pos]; }
	Token& last();

	std::vector<Token> m_tokens;
	uint32_t m_pos{ 0 }, m_dataPtr{ 0 };

	std::map<std::string, uint32_t> m_labels;
	std::map<std::string, uint32_t> m_refs;

	Console *m_console;

	Scanner m_scanner;
};

#endif // ASM_H
