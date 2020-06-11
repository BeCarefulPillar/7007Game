#include <iostream>
#include <memory>
#include <stdio.h>

class ClassA {
    int a = 5;
    int b = 5;
public:
    ClassA() {

    }
    ~ClassA() {

    }

private:
    
};

int main()
{
//     char* b = new char[12];
//     char* c = new char;
//     delete[] b;
//     char* a = new char[4];
//     delete[] a;
//     delete c;

    std::shared_ptr<ClassA>b = std::make_shared<ClassA>();


    //     std::cout << "Hello World!\n";
    return 0;
}