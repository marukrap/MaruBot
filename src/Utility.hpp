#pragma once

#include <SFML/Graphics/Text.hpp>

// random number generation
int randomInt(int exclusiveMax); // [0, max)
int randomInt(int min, int inclusiveMax); // [min, max]

// string operations
bool startsWith(const std::string& str, const std::string& prefix);
bool endsWith(const std::string& str, const std::string& suffix);

void toLower(std::string& str);
void toUpper(std::string& str);

std::string trim(const std::string& str);
std::string getSubstring(const std::string& str, const std::string& tokenLeft, const std::string& tokenRight = "");

std::wstring ansiToUtf16(const std::string& ansi);
std::wstring utf8ToUtf16(const std::string& utf8);

std::string utf16ToAnsi(const std::wstring& utf16);
std::string utf16ToUtf8(const std::wstring& utf16);

std::string ansiToUtf8(const std::string& ansi);
std::string utf8ToAnsi(const std::string& utf8);

// execute a file
std::string exec(const std::string& cmd);
std::string exec(const std::wstring& cmd);

// center objects
void centerOrigin(sf::Text& text);
