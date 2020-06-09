#include <iostream>

int main()
{
    char* b = new char[12];
    char* c = new char;
    delete[] b;
    char* a = new char[5];
    delete[] a;
    delete(c);

//     std::cout << "Hello World!\n";
    return 0;
}