#include <iostream>
#include <vector>
#include <cmath>
#include <memory>

using namespace std;


struct Expression {
	float value;
	virtual void evaluate() = 0;
	virtual void derive(float seed = 1.0) = 0;
};

using ptrExp = std::shared_ptr<Expression>;


struct Variable: public Expression {
	float partial = 0;

	Variable(float v) {
		value = v;
	}

	void evaluate() {}

	void derive(float seed = 1.0) {
		partial += seed;
	}
};


struct Plus: public Expression {
	ptrExp a, b;

	Plus(ptrExp a, ptrExp b) : a(a), b(b) {

	}

	void evaluate() {
		a->evaluate();
		b->evaluate();

		value = a->value + b->value;
	}

	void derive(float seed = 1.0) {
		a->derive(seed);
		b->derive(seed);
	}

};


struct Minus: public Expression {
	ptrExp a, b;

	Minus(ptrExp a, ptrExp b) : a(a), b(b) {

	}

	void evaluate() {
		a->evaluate();
		b->evaluate();

		value = a->value - b->value;
	}

	void derive(float seed = 1.0) {
		a->derive(seed);
		b->derive(-seed);
	}

};


struct Multiply: public Expression {
	ptrExp a, b;

	Multiply(ptrExp a, ptrExp b) : a(a), b(b) {

	}

	void evaluate() {
		a->evaluate();
		b->evaluate();

		value = a->value * b->value;
	}

	void derive(float seed = 1.0) {
		a->derive(b->value * seed);
		b->derive(a->value * seed);
	}
};




ptrExp operator * (ptrExp exp1, ptrExp exp2) {
	return std::make_shared<Multiply>(exp1, exp2);
}

ptrExp operator + (ptrExp exp1, ptrExp exp2) {
	return std::make_shared<Plus>(exp1, exp2);
}




struct Vector {
	vector<ptrExp> data;
	size_t size;

	Vector(vector<float> vec) {
		for (size_t i = 0; i < vec.size(); ++i) {
			data.push_back(std::make_shared<Variable>(vec[i]));
		}

		size = vec.size();
	}
	Vector(size_t s, float fillValue = 0) {
		for (size_t i = 0; i < s; ++i) {
			data.push_back(std::make_shared<Variable>(fillValue));
		}

		size = s;
	}

	ptrExp dot(const Vector& other) {
		ptrExp f = std::make_shared<Variable>(0.0);

		for (size_t i = 0; i < size; ++i) {
			f = f + data[i] * other.data[i];
		}

		return f;
	}

	vector<float> getGradient() {
		vector<float> ans(size);

		for (size_t i = 0; i < size; ++i) {
			ans[i] = std::dynamic_pointer_cast<Variable>(data[i])->partial;
		}

		return ans;
	}

};










int main () {

	Vector a({ 1.0, 1.0, 1.0, 1.0, 1.0 });

	ptrExp g = a.data[0] * std::make_shared<Variable>(3.0f);
	g->evaluate();
	g->derive();
	cout << std::dynamic_pointer_cast<Variable>(a.data[0])->partial << "\n";;

	ptrExp f = a.dot(a);

	f->evaluate();
	f->derive();
	vector<float> grad = a.getGradient();

	cout << "Grad: ";
	for (size_t i = 0; i < grad.size(); ++i) {
		cout << grad[i] << ", ";
	}
	cout << "\n";

	return 0;
}
