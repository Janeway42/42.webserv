#include <iostream>

int main()
{
	std::string input = "0123 567890 23 5678 \r\n\r\n";
	std::string tofind = "\r\n\r\n";
	int loc = input.find(tofind);
	std::string before = input.substr(0, loc);
	std::string found = input.substr(loc);

	std::cout << "loc: " << loc  << std::endl;
	std::cout << "before: " << before << "|" << std::endl;
	std::cout << "found: " << "|" << found << "|" << std::endl;
	return (0);
}