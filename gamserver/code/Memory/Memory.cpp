#include <iostream>
#include <memory>
#include <stdio.h>

#include "CellObjPool.hpp"
class ClassA :public ObjectPoolBase<ClassA,5>{
    int _a;
public:
    ClassA(int a) {
        this->_a = a;
        printf("ClassA\n");
    }
    ~ClassA() {
        printf("~ClassA\n");
    }

private:
    
};

int main() {

    ClassA* a[6];
    for (int i = 0; i < 6; i++) {
        a[i] = new ClassA(3);
    }
    for (int i = 0; i < 6; i++) {
        delete a[i];
    }


//     char* b = new char[12];
//     char* c = new char;
//     delete[] b;
//     char* a = new char[4];
//     delete[] a;
//     delete c;

 //   std::shared_ptr<ClassA>b = std::make_shared<ClassA>();


    //     std::cout << "Hello World!\n";
    return 0;
}