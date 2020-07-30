#include <iostream>
using namespace std;

int main() {
    auto n = 0;
    try {
        cout << "before dividing." << endl;
        if (n == 0)
            throw 10; //抛出int类型异常
        else
            cout <<  n << endl;
        cout << "after dividing." << endl;
    } catch (double d) {
        cout << "catch(double) " << d << endl;
    } catch (int e) {
        cout << "catch(int) " << e << endl;
    } catch (...) {
        cout << "catch(...) "  << endl;
    }
    cout << "finished" << endl;
    return 0;
}