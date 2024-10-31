#include "../graph.h"
#include "operations.hpp"

#include <iostream>

using namespace std;

#define EPS 1e-4

// gotta use doubles, floats couldn't handle it
double testFunction(double x) {
	return std::sqrt(std::exp(std::sin(std::sin(std::cos(x)))));
}

double testDerivative(double x, int n = 1) {
	if (n == 1) {
		return (testFunction(x + EPS) - testFunction(x - EPS)) / (2.0 * EPS);
	}

	return (testDerivative(x + EPS, n - 1) - testDerivative(x - EPS, n - 1)) / (2.0 * EPS);
}


int main() {

	Var x = Scalar::build(0.0f);

	Var f = sqrt(exp(sin(sin(cos(x)))));

	f->calculateGradientFunctions();
	Var grad = x->gradientFunction; // first order

	grad->calculateGradientFunctions();
	grad = x->gradientFunction; // second order

	grad->calculateGradientFunctions();
	grad = x->gradientFunction; // third order


	Line func(olc::RED);
	Line derivative(olc::BLUE);
	Line derivative2(olc::GREEN);

	// compare with finite-difference just to be shure (it does struggle with precision)
	for (float i = -5.0f; i <= 5.0f; i += 0.1f) {
		x->value = i;

		f->eval();
		grad->eval();

		func.addPoint(Point(i, f->value));
		derivative.addPoint(Point(i, grad->value));
		derivative2.addPoint(Point(i, testDerivative(i, 3)));
	}

	Graph graph;
	graph.addLine(func);
	graph.addLine(derivative);
	graph.addLine(derivative2);

	graph.waitFinish();

	return 0;
}
