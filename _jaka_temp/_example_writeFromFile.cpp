

// Example: https://en.cppreference.com/w/cpp/io/basic_ifstream
#include <iostream>
#include <fstream>
#include <string>
 
int main()
{
    std::string filename = "temp.txt";
 
    // prepare a file to read
    double d = 3.14;
    std::ofstream(filename, std::ios::binary).write(reinterpret_cast<char*>(&d), sizeof d) << 123 << "abc";
 
    // open file for reading
    std::ifstream istream(filename, std::ios::binary);
    if (!istream.is_open())
        std::cout << "failed to open " << filename << '\n';
    else
    {
        double d;
        istream.read(reinterpret_cast<char*>(&d), sizeof d); // binary input
        int n;
        std::string s;
        if (istream >> n >> s)                               // text input
            std::cout << "read back from file: " << d << ' ' << n << ' ' << s << '\n';
    }
}








/* EXAMPLE https://codescracker.com/cpp/program/cpp-program-read-and-display-file.htm
#include<iostream>
#include<fstream>
#include<stdio.h>

//using namespace std;
int main()
{
    char ch;
    std::fstream fp;
    // cout<<"Enter the Name of File: ";
    // gets(fileName);
    fp.open("temp.txt", std::fstream::in);
    if(!fp)
    {
        std::cout << "\nError Occurred!";
        return 0;
    }
    std::cout << "\nContent of "<< "temp.txt" << ": \n";
    while(fp >> std::noskipws >> ch)
        std::cout << ch;
    fp.close();
    std::cout << std::endl;
    return 0;
}
*/