#include <iostream>
#include <string>
#include <vector>

class Ex {
public:
	int i;
	std::string str;
	std::vector<int> vec_in;

	Ex() {
		i = 3;
		std::cout << "i: " << i << std::endl;
		str = "joyce";
		std::cout << "str: " << str << std::endl;
		vec_in.push_back(3);
		vec_in.push_back(2);
		std::cout << "vec size: " << vec_in.size() << std::endl << std::endl;
	}
	~Ex() {};

};

int main() {
	Ex exemple;
	exemple.~Ex();

	std::cout << "After destructor -> i: " << exemple.i << std::endl;
	std::cout << "After destructor -> str: " << exemple.str << std::endl;
	std::cout << "After destructor -> vec size: " << exemple.vec_in.size() << std::endl << std::endl;

	return 0;
}
