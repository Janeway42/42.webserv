#include <iostream>
#include <string>
#include <climits>
 
int main()
{
    std::string s;
    std::cout
        << "Maximum size of a string is " << s.max_size() << " ("
        << std::hex << std::showbase << s.max_size()
        << "), pointer size: " << std::dec
        << CHAR_BIT*sizeof(void*) << " bits\n";
}