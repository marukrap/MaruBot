#include "Utility.hpp"

#include <Windows.h>

#include <random>
#include <cctype>
#include <memory>
#include <array>

namespace
{
	std::random_device rd;
	std::mt19937 mt(rd());
}

int randomInt(int exclusiveMax)
{
	return std::uniform_int_distribution<>(0, exclusiveMax - 1)(mt);
}

int randomInt(int min, int inclusiveMax)
{
	return min + std::uniform_int_distribution<>(0, inclusiveMax - min)(mt);
}

bool startsWith(const std::string& str, const std::string& prefix)
{
	if (prefix.size() > str.size())
		return false;

	return str.compare(0, prefix.size(), prefix) == 0;
}

bool endsWith(const std::string& str, const std::string& suffix)
{
	if (suffix.size() > str.size())
		return false;

	return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

void toLower(std::string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), [] (unsigned char ch) { return std::tolower(ch); });
}

void toUpper(std::string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), [] (unsigned char ch) { return std::toupper(ch); });
}

std::string trim(const std::string& str)
{
	const std::size_t first = str.find_first_not_of(" \t\n");
	const std::size_t last = str.find_last_not_of(" \t\n");

	if (first == std::string::npos)
		return "";

	return str.substr(first, last - first + 1);
}

std::string getSubstring(const std::string& str, const std::string& tokenLeft, const std::string& tokenRight)
{
	if (std::size_t begin = str.find(tokenLeft); begin != std::string::npos)
	{
		begin += tokenLeft.size();

		if (!tokenRight.empty())
			return str.substr(begin, str.find(tokenRight, begin) - begin);
		else
			return str.substr(begin);
	}

	return "";
}

std::wstring ansiToUtf16(const std::string& ansi)
{
	const int utf16Length = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, ansi.data(), ansi.length(), nullptr, 0);
	std::wstring utf16(utf16Length, L'\0');
	MultiByteToWideChar(CP_ACP, MB_COMPOSITE, ansi.data(), ansi.length(), &utf16[0], utf16Length);

	return utf16;
}

std::wstring utf8ToUtf16(const std::string& utf8)
{
	const int utf16Length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), utf8.length(), nullptr, 0);
	std::wstring utf16(utf16Length, L'\0');
	MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), utf8.length(), &utf16[0], utf16Length);

	return utf16;
}

std::string utf16ToAnsi(const std::wstring& utf16)
{
	const int ansiLength = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, utf16.data(), utf16.length(), nullptr, 0, nullptr, nullptr);
	std::string ansi(ansiLength, '\0');
	WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, utf16.data(), utf16.length(), &ansi[0], ansiLength, nullptr, nullptr);

	return ansi;
}

std::string utf16ToUtf8(const std::wstring& utf16)
{
	const int utf8Length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16.data(), utf16.length(), nullptr, 0, nullptr, nullptr);
	std::string utf8(utf8Length, '\0');
	WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, utf16.data(), utf16.length(), &utf8[0], utf8Length, nullptr, nullptr);

	return utf8;
}

std::string ansiToUtf8(const std::string& ansi)
{
	return utf16ToUtf8(ansiToUtf16(ansi));
}

std::string utf8ToAnsi(const std::string& utf8)
{
	return utf16ToAnsi(utf8ToUtf16(utf8));
}

std::string exec(const std::string& cmd)
{
	// https://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
	// https://stackoverflow.com/questions/21575310/converting-normal-stdstring-to-utf-8

	std::shared_ptr<FILE> pipe(_popen(cmd.data(), "r"), _pclose);

	if (!pipe)
		throw std::runtime_error("_popen() failed!");

	std::array<char, 128> buffer;
	std::string result;

	while (!feof(pipe.get()))
	{
		if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
			result += buffer.data();
	}

	return result; // utf8
}

std::string exec(const std::wstring& cmd)
{
	std::shared_ptr<FILE> pipe(_wpopen(cmd.data(), L"r"), _pclose);

	if (!pipe)
		throw std::runtime_error("_popen() failed!");

	std::array<char, 128> buffer;
	std::string result;

	while (!feof(pipe.get()))
	{
		if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
			result += buffer.data();
	}

	return result;
}

void centerOrigin(sf::Text& text)
{
	const sf::FloatRect bounds = text.getLocalBounds();

	text.setOrigin(std::floor(bounds.left + bounds.width / 2), std::floor(bounds.top + bounds.height / 2));
}
