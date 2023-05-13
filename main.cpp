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
		//std::cout << "i: " << i << std::endl;
		str = "joyce";
		//std::cout << "str: " << str << std::endl;

		const char *st = "abc";
		size_t len = 2;
		std::vector<uint8_t> tempVec(st, st + len);
		std::cout << "st + len: " << st + len << std::endl << std::endl;
		// 0x45665456
		// 0x45665459

		std::vector<uint8_t>::iterator it = tempVec.begin();
		std::cout << "tempVec size: " << tempVec.size() << std::endl << std::endl;

		for (; it < tempVec.end(); it++) {
			std::cout << "tempVec it: " << *it << std::endl << std::endl;
		}
		std::cout << "tempVec size: " << tempVec.size() << std::endl << std::endl;


		vec_in.push_back(3);
		vec_in.push_back(2);
		//std::cout << "vec size: " << vec_in.size() << std::endl << std::endl;
	}
	~Ex() {};

};

int main() {
	Ex exemple;
	//exemple.~Ex();

	//std::cout << "After destructor -> i: " << exemple.i << std::endl;
	//std::cout << "After destructor -> str: " << exemple.str << std::endl;
	//std::cout << "After destructor -> vec size: " << exemple.vec_in.size() << std::endl << std::endl;

	return 0;
}
