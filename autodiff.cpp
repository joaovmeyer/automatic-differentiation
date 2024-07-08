
#include "operations.hpp"

#include <iostream>

using namespace std;


int main() {

    Var a1 = Scalar::build(2.0f, "a1");
    Var a2 = Scalar::build(6.0f, "a2");

    Var func = (a1 + a2) * a2;

    func->calculateDerivatives();

    cout << "Function: " << func->name << "\n";
    cout << "Value at (a1, a2) = (2, 6): " << func->value << "\n";
    cout << "Derivatives: \n - a1: " << a1->partial << "\n - a2: " << a2->partial << "\n\n";



    Var x = Scalar::build(1.0f, "x");

    Var f = Sin::build(x) + Cos::build(x * x) + Exp::build(x * x * x) + Ln::build(x + 1.0f) + 1.0f / (x + 2.0f);

    f->calculateDerivatives();

    cout << "Function: " << f->name << "\n";
    cout << "Value at x = 1: " << f->value << "\n";
    cout << "Derivatives: \n - x: " << x->partial << "\n\n";



    Vec v1 = Vector::build(5, 1.2f, "v1");
    Vec v2 = Vector::build(5, 0.3f, "v2");

    Var dot = v1 * v2;

    dot->calculateDerivatives();

    cout << "Function: " << dot->name << "\n";
    cout << "Value at v1 = " << v1->value << ", v2 = " << v2->value << ": " << dot->value << "\n";
    cout << "Derivatives: \n - v1: " << v1->partial << "\n";
    cout << " - v2: " << v2->partial << "\n\n";

	return 0;
}
