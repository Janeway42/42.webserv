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

    /** initialized "manually" */

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

    Test test{};

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
