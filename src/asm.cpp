#include "asm.h"

#include <iostream>
#include <cctype>
#include <regex>

#define error(x) std::cerr << x << std::endl

Scanner::Scanner(const std::string& input)
	: m_input(input), m_pos(0)
{
//	std::cout << "INPUT:\n" << input << std::endl;
}

char Scanner::next() {
	if (m_pos >= m_input.size()) return '\0';
	return m_input[m_pos++];
}

char Scanner::peek() const {
	if (m_pos+1 >= m_input.size()) return '\0';
	return m_input[m_pos + 1];
}

char Scanner::prev() const {
	if (m_pos - 1 < 0) return '\0';
	return m_input[m_pos - 1];
}

void Scanner::advance(int n) {
	if (m_pos+n >= m_input.size()) return;
	m_pos += n;
}

// -------------- ASM ---------------

ASM::ASM(const std::string& input, Console *console)
	: m_scanner(Scanner(input)), m_console(console)
{}

void ASM::tokenize() {
	m_tokens.clear();

	const std::regex symbolRE("[-!$%^&*()_+|~=`{}\\[\\]:<>?,.\\/\\\\]");

#define C m_scanner.current()
#define P m_scanner.prev()
#define N m_scanner.peek()
#define S std::string(1, C)

	while (m_scanner.hasNext()) {
		if (std::isalpha(C) || C == '_' || C == '&') { // TokIdentifier/TokRef/TokNewLabel
			std::string res = "";
			while ((std::isalnum(C) || C == '_' || C == '&' || C == ':') && m_scanner.hasNext()) {
				res += C;
				m_scanner.next();
			}

			std::string rlow = res;
			std::transform(rlow.begin(), rlow.end(), rlow.begin(), ::tolower);

			TokenType type = TokenType::TokIdentifier;
			Byte value = 0;
			if (res[0] == '&') {
				type = TokenType::TokReference;
				res.erase(0, 1);
			} else if (res[res.size() - 1] == ':') {
				type = TokenType::TokNewLabel;
				res.erase(res.size() - 1, 1);
			} else if (rlow == "let") {
				type = TokenType::TokLet;
			} else if (OP_CODES.find(rlow) != OP_CODES.end()) {
				type = TokenType::TokOpCode;
				value = OP_CODES[rlow];
			}
			m_tokens.push_back(Token(type, res, value));
		} else if (std::isdigit(C)) {
			std::string res = "";
			int base = 10;
			if (C == '0' && (N == 'x' || N == 'X')) {
				m_scanner.next();
				m_scanner.next();
				base = 16;
			}

			while ((std::isdigit(C) || std::isalpha(C)) && m_scanner.hasNext()) {
				res += C;
				m_scanner.next();
			}
			m_tokens.push_back(Token(TokenType::TokNumber, res, Byte(std::stoi(res, nullptr, base))));
		} else if (C == '[') {
			m_tokens.push_back(Token(TokenType::TokOpenBracket, "[", 0));
			m_scanner.next();
		} else if (C == ']') {
			m_tokens.push_back(Token(TokenType::TokCloseBracket, "]", 0));
			m_scanner.next();
		} else if (C == ',') {
			m_tokens.push_back(Token(TokenType::TokComma, ",", 0));
			m_scanner.next();
		} else if (C == ';') {
			while (C != '\n' && C != '\r' && m_scanner.hasNext()) {
				m_scanner.next();
			}
		} else {
			m_scanner.next();
		}
	}

//	printTokens();
}

ByteList ASM::compile() {
	ByteList code;

	tokenize();
	readLabelsAndRefs();

	while (m_pos < m_tokens.size()) {
		ByteList part = instruction();
		code.insert(code.end(), part.begin(), part.end());
	}

	return code;
}

void ASM::printTokens() {
	for (auto&& tok : m_tokens) {
		std::cout << tok.toString() << " ";
	}
	std::cout << std::endl;
}

bool ASM::accept(TokenType type, const std::string& param, bool regex, bool forceCheck) {
	if (m_pos >= m_tokens.size()) return false;
	if (current().type == type) {
		if (forceCheck) {
			bool test = regex ? std::regex_match(current().lexeme, std::regex(param)) : current().lexeme == param;
			if (test) return next();
			else return false;
		}
		return next();
	}
	return false;
}

bool ASM::expect(TokenType type, const std::string& param, bool regex, bool forceCheck) {
	if (accept(type, param, regex, forceCheck)) {
		return true;
	}

	std::string expc = "Unknown";
	switch (type) {
		case TokenType::TokIdentifier: expc = "Identifier"; break;
		case TokenType::TokNumber: expc = "Number"; break;
		case TokenType::TokReference: expc = "Reference"; break;
		case TokenType::TokNewLabel: expc = "New Label"; break;
		case TokenType::TokOpenBracket: expc = "Open Bracket"; break;
		case TokenType::TokCloseBracket: expc = "Close Bracket"; break;
		case TokenType::TokComma: expc = "Comma"; break;
		default: break;
	}

	error(
		"ERROR: Unexpected symbol \"" <<
		current().lexeme <<
		"\". Expected \"" <<
		expc <<
		"\" Near \"" <<
		last().lexeme <<
		"\"."
	);
	return false;
}

bool ASM::next() {
	if (m_pos >= m_tokens.size()) return false;
	m_pos++;
	return true;
}

void ASM::stepBack() {
	if (m_pos - 1 < 0) return;
	m_pos--;
}

Token& ASM::last() {
	if (m_pos - 1 < 0) return m_tokens[m_pos];
	return m_tokens[m_pos - 1];
}

void ASM::readLabelsAndRefs() {
	std::vector<uint32_t> remove;
	for (uint32_t i = 0; i < m_tokens.size(); i++) {
		Token tok = m_tokens[i];
		if (tok.type == TokLet) {
			remove.push_back(i);
			i++;
			Token vn = m_tokens[i];
			if (vn.type == TokenType::TokIdentifier) {
				std::string varName = vn.lexeme;
				remove.push_back(i);
				i++;
				Token cm = m_tokens[i];
				if (cm.type == TokenType::TokComma) {
					remove.push_back(i);
					i++;
					Token nb = m_tokens[i];
					ByteList params;
					if (nb.type == TokenType::TokNumber) {
						remove.push_back(i);
						params.push_back(m_tokens[i].value);
					} else if (nb.type == TokenType::TokOpenBracket) {
						remove.push_back(i);
						i++;
						Token curr = m_tokens[i];
						while (curr.type != TokenType::TokCloseBracket) {
							if (curr.type == TokenType::TokNumber) {
								params.push_back(curr.value);
								remove.push_back(i);
								i++;
								if (m_tokens[i].type == TokenType::TokCloseBracket) {
									i--;
								} else if (m_tokens[i].type != TokenType::TokComma) {
									error("ERROR: Expected a Comma after a number in the Data Block.");
									error("At \"" << m_tokens[i-1].lexeme << "\"");
									error("Before \"" << m_tokens.back().lexeme << "\"");
									break;
								} else {
									remove.push_back(i);
								}
							}
							i++;
							curr = m_tokens[i];
						}
						remove.push_back(i);
					}
					m_refs[varName] = m_dataPtr;
					for (Byte b : params) {
						m_console->data()[m_dataPtr++] = b;
					}
				} else {
					m_refs[varName] = m_dataPtr;
					m_dataPtr++;
				}
				i--;
			}
		}
	}

	std::reverse(remove.begin(), remove.end());
	for (auto&& i : remove)
		m_tokens.erase(m_tokens.begin() + i);
	remove.clear();

	uint32_t pos = 0, i = 0;
	for (auto&& tok : m_tokens) {
		if (tok.type == TokIdentifier ||
			tok.type == TokNumber ||
			tok.type == TokOpCode ||
			tok.type == TokReference ||
			tok.type == TokIdentifier)
			pos++;
		else if (tok.type == TokNewLabel) {
			m_labels[tok.lexeme] = pos;
			remove.push_back(i);
		}
		i++;
	}

	std::reverse(remove.begin(), remove.end());
	for (auto&& i : remove)
		m_tokens.erase(m_tokens.begin() + i);

}

ByteList ASM::atom() {
	ByteList ret;
	if (accept(TokenType::TokIdentifier)) {
		Token tok = last();
		ret.push_back(m_labels[tok.lexeme]);
		return ret;
	} else if (accept(TokenType::TokNumber)) {
		ret.push_back(last().value);
		return ret;
	} else if (accept(TokenType::TokReference)) {
		ret.push_back(m_refs[last().lexeme]);
		return ret;
	}
	return ret;
}

ByteList ASM::instruction() {
	ByteList ret;
	if (expect(TokenType::TokOpCode)) {
		Token tok = last();
		ret.push_back(tok.value);
		ByteList a = atom();
		if (!a.empty()) ret.insert(ret.end(), a.begin(), a.end());
		while (accept(TokenType::TokComma)) {
			ByteList rest = atom();
			ret.insert(ret.end(), rest.begin(), rest.end());
		}
	}
	return ret;
}
