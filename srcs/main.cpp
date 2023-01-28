#include <fstream>
// #include "../includes/ConfigFileParser.hpp"

int main(int ac, char **av, char **env) {
    (void)env;

    // ADDED JAKA
    data::RequestFile requestFile;

	if (requestFile.handleFile("incomingRequest"))
	{
		// STORE FIRST LINE HEADER
		std::string reqMethodGET = requestFile.getRequestData().getRequestMethod();
		std::string reqHttpPath = requestFile.getRequestData().getHttpPath();
		std::string reqHttpVersion = requestFile.getRequestData().getHttpVersion();

		std::cout << REDB << "Found first line header: [" << reqMethodGET << ", " << reqHttpPath << ", " << reqHttpVersion << "]" << BACK << std::endl;

		// STORE OTHER HEADERS
		std::string reqHost = requestFile.getRequestData().getRequestHost();
		std::cout << REDB << "Found request Host: [" << reqHost << "]" << BACK << std::endl;

		std::string reqAccept = requestFile.getRequestData().getRequestAccept();
		std::cout << REDB << "Found request Accept: [" << reqAccept << "]" << BACK << std::endl;

		// STORE BODY
	}

    return EXIT_SUCCESS;
}


/*
#include <iostream>

class Test {
public:
    struct {
        int test_var;
    } test_struct_untagged; // not a type, so it can't be used to make a copy of this struct

    struct name {
        int test_var;
    } test_struct_tagged;
    struct name test_struct_tagged_second_declaration;

    struct same_name {
        int test_var;
    } same_name;

    // initialized "manually"

    struct test_struct_tagged_not_declared_2 {
        int test_var;
    };
    test_struct_tagged_not_declared_2 test_struct_tagged_declared_2;

};

struct test_struct_tagged_not_declared {
    int test_var;
};// not declaring
// also not declaring "manually

int main() {

    Test test;

    test.test_struct_untagged.test_var = 1;
    std::cout << test.test_struct_untagged.test_var << std::endl;

    test.test_struct_tagged.test_var = 2;
    std::cout << test.test_struct_tagged.test_var << std::endl;

    test.same_name.test_var = 3;
    std::cout << test.same_name.test_var << std::endl;

    test.test_struct_tagged_declared_2.test_var = 4;
    std::cout << test.test_struct_tagged_declared_2.test_var << std::endl;

    struct test_struct_tagged_not_declared joyce = {};
    joyce.test_var = 5;
    std::cout << joyce.test_var << std::endl;
    return 0;
}
*/


