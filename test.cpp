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

void operator -= (const ptrExp& exp, float val) {
	exp->value -= val;
}



struct Variable: public Expression {
	float partial = 0.0;

	Variable(float v) {
		value = v;
	}

	void evaluate() { partial = 0.0; } // don't actually think this should be here

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


struct Sigmoid: public Expression {
	ptrExp a;

	Sigmoid(ptrExp a) : a(a) {

	}

	void evaluate() {
		a->evaluate();

		value = 1.0 / (1.0 + std::exp(-a->value));
	}

	void derive(float seed = 1.0) {
		float da = value * (1.0 - value);
		a->derive(da * seed);
	}
};



ptrExp operator * (const ptrExp& exp1, const ptrExp& exp2) {
	return std::make_shared<Multiply>(exp1, exp2);
}

ptrExp operator + (const ptrExp& exp1, const ptrExp& exp2) {
	return std::make_shared<Plus>(exp1, exp2);
}

ptrExp operator - (const ptrExp& exp1, const ptrExp& exp2) {
	return std::make_shared<Minus>(exp1, exp2);
}

ptrExp sigmoid(const ptrExp& x) {
	return std::make_shared<Sigmoid>(x);
}



struct Vec {
	vector<ptrExp> data;
	size_t size;

	Vec(const vector<float>& vec) {
		for (size_t i = 0; i < vec.size(); ++i) {
			data.push_back(std::make_shared<Variable>(vec[i]));
		}

		size = vec.size();
	}
	Vec(size_t s, float fillValue = 0) {
		for (size_t i = 0; i < s; ++i) {
			data.push_back(std::make_shared<Variable>(fillValue));
		}

		size = s;
	}


	ptrExp& operator [] (int i) {
		return data[i];
	}

	// read only
	ptrExp operator [] (int i) const {
		return data[i];
	}







	static ptrExp dot(const Vec& v1, const Vec& v2) {
		ptrExp f = std::make_shared<Variable>(0.0);

		for (size_t i = 0; i < v1.size; ++i) {
			f = f + v1[i] * v2[i];
		}

		return f;
	}

	static Vec hadamard(const Vec& v1, const Vec& v2) {
		Vec ans(v1.size);

		for (size_t i = 0; i < v1.size; ++i) {
			ans[i] = v1[i] * v2[i];
		}

		return ans;
	}

	static Vec sigmoid(const Vec& v) {
		Vec ans(v.size);

		for (size_t i = 0; i < v.size; ++i) {
			ans[i] = std::make_shared<Sigmoid>(v[i]);
		}

		return ans;

	}

	vector<float> getGradient() {
		vector<float> grad(size);

		for (size_t i = 0; i < size; ++i) {
			grad[i] = std::dynamic_pointer_cast<Variable>(data[i])->partial;
		}

		return grad;
	}





	Vec operator + (const Vec& other) const {
		Vec ans(size, 0.0);

		for (size_t i = 0; i < size; ++i) {
			ans[i] = data[i] + other[i];
		}

		return ans;
	}

	Vec operator - (const Vec& other) const {
		Vec ans(size, 0.0);

		for (size_t i = 0; i < size; ++i) {
			ans[i] = data[i] - other[i];
		}

		return ans;
	}

};



struct Mat {
	vector<Vec> data;
	size_t rows, cols;


	Mat(const vector<vector<float>>& mat) {
		for (size_t i = 0; i < mat.size(); ++i) {
			data.push_back(Vec(mat[i].size(), 0.0f));

			for (size_t j = 0; j < mat[i].size(); ++j){
				data[i][j] = std::make_shared<Variable>(mat[i][j]);
			}
		}

		rows = mat.size();
		cols = mat[0].size();
	}

	Mat(size_t c, size_t r, float fillValue = 0) {
		for (size_t i = 0; i < r; ++i) {
			data.push_back(Vec(c, fillValue));
		}

		rows = r;
		cols = c;
	}


	Vec& operator [] (int i) {
		return data[i];
	}

	// read only
	Vec operator [] (int i) const {
		return data[i];
	}


	Vec operator * (const Vec& v) {
		Vec ans(rows, 0.0);

		for (size_t i = 0; i < rows; ++i) {
			ans[i] = Vec::dot(data[i], v);
		}

		return ans;
	}


	vector<vector<float>> getGradient() {
		vector<vector<float>> grad(rows, vector<float>(cols, 0.0));

		for (size_t i = 0; i < rows; ++i) {
			for (size_t j = 0; j < cols; ++j) {
				grad[i][j] = std::dynamic_pointer_cast<Variable>(data[i][j])->partial;
			}
		}

		return grad;
	}
};










int main () {

	Vec x(2, 1.0);
	Vec y(1, 0.0);

	Mat w1({
		{ 0.1, 0.2 },
		{ 0.4, 0.3 }
	});
	Vec b1(2, 0.1);

	Mat w2({
		{ 0.6, 0.5 }
	});
	Vec b2(1, 0.1);

	Vec a1 = Vec::sigmoid(w1 * x + b1);
	Vec y_pred = Vec::sigmoid(w2 * a1 + b2);

	ptrExp loss = Vec::dot(y_pred - y, y_pred - y);

	y_pred[0]->evaluate();
	loss->evaluate();
	cout << "Before training: \n";
	cout << "Value: " << y_pred[0]->value << ", expected: 0.0\n";
	cout << "Loss: " << loss->value << "\n";

	float lr = 0.1;

	for (int i = 0; i < 100000; ++i) {
		loss->evaluate();
		loss->derive();

		vector<float> gradB1 = b1.getGradient();
		vector<float> gradB2 = b2.getGradient();

		vector<vector<float>> gradW1 = w1.getGradient();
		vector<vector<float>> gradW2 = w2.getGradient();

		for (size_t i = 0; i < b1.size; ++i) {
			b1[i] -= gradB1[i] * lr;

			for (size_t j = 0; j < w1.cols; ++j) {
				w1[i][j] -= gradW1[i][j] * lr;
			}
		}

		for (size_t i = 0; i < b2.size; ++i) {
			b2[i] -= gradB2[i] * lr;

			for (size_t j = 0; j < w2.cols; ++j) {
				w2[i][j] -= gradW2[i][j] * lr;
			}
		}
	}

	y_pred[0]->evaluate();
	loss->evaluate();
	cout << "Before training: \n";
	cout << "Value: " << y_pred[0]->value << ", expected: 0.0\n";
	cout << "Loss: " << loss->value << "\n";


	return 0;
}
