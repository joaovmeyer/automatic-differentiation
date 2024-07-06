#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <string>
#include <functional>
#include <cmath>


using namespace std;



struct Node : enable_shared_from_this<Node> {
	vector<shared_ptr<Node>> parents;
	string name;

	Node(string n = "") : name(n) {

	}

    virtual inline void evaluate() = 0;
    virtual inline void derive() = 0;


    vector<std::shared_ptr<Node>> topologicalSort() {
    	vector<std::shared_ptr<Node>> ordering;
		unordered_set<std::shared_ptr<Node>> visited;

		std::function<void(const std::shared_ptr<Node>&)> addChildren = [&](const std::shared_ptr<Node>& node) {
			// if the node has no parents, we shouldn't need to either evaluate nor derive it
			if (visited.find(node) != visited.end() || !node->parents.size()) {
				return;
			}

			visited.insert(node);
			for (size_t i = 0; i < node->parents.size(); ++i) {
				addChildren(node->parents[i]);
			}

			ordering.push_back(node);
		};

		// this does require this Node to have a shared_ptr to it, 
		// but there's no situation where it shouldn't have one, so it's fine
		addChildren(shared_from_this());

		return ordering;
	}
};


struct Variable : Node {

    float value;
    float partial;

    Variable(float v = 0.0f, string n = "") : value(v), partial(0.0f) {
        name = (n == "") ? std::to_string(v) : n;
    }

    static std::shared_ptr<Variable> build(float v = 0.0f, string n = "") {
        return std::make_shared<Variable>(v, n);
    }

    void evaluate() override {

    }

    void derive() override {

    }

    void calculateDerivatives() {
    	vector<std::shared_ptr<Node>> ordering = topologicalSort();

	    for (size_t i = 0; i < ordering.size(); ++i) {
	        ordering[i]->evaluate();
		}

	    partial = 1.0; // dx/dx is 1 for whatever x
	    for (size_t i = ordering.size(); i > 0; --i) {
	        ordering[i - 1]->derive();
		}
    }
};

using Var = std::shared_ptr<Variable>;

struct Add : Variable {

    Var a, b;

    Add() {}

    static Var build(const Var& v1, const Var& v2) {

        shared_ptr<Add> add = make_shared<Add>();

        add->a = v1;
        add->b = v2;
        add->name = "(" + v1->name + " + " + v2->name + ")";

        add->parents.push_back(v1);
        add->parents.push_back(v2);

        return add;
    }

    void evaluate() override final {
        value = a->value + b->value;
    }

    void derive() override final {
        a->partial += partial;
        b->partial += partial;
    }
};

struct Mult : Variable {

    Var a, b;

    Mult() {}

    static Var build(const Var& v1, const Var& v2) {

        shared_ptr<Mult> mult = make_shared<Mult>();

        mult->a = v1;
        mult->b = v2;
        mult->name = "(" + v1->name + " * " + v2->name + ")";

        mult->parents.push_back(v1);
        mult->parents.push_back(v2);

        return mult;
    }

    void evaluate() override final {
        value = a->value * b->value;
    }

    void derive() override final {
        a->partial += partial * b->value;
        b->partial += partial * a->value;
    }
};

struct Sin : Variable {

    Var a;

    Sin() {}

    static Var build(const Var& v) {

        shared_ptr<Sin> sin = make_shared<Sin>();

        sin->a = v;
        sin->name = "sin(" + v->name + ")";

        sin->parents.push_back(v);

        return sin;
    }

    void evaluate() override final {
        value = std::sin(a->value);
    }

    void derive() override final {
        a->partial += partial * std::cos(a->value);
    }
};

struct Cos : Variable {

    Var a;

    Cos() {}

    static Var build(const Var& v) {

        shared_ptr<Cos> cos = make_shared<Cos>();

        cos->a = v;
        cos->name = "cos(" + v->name + ")";

        cos->parents.push_back(v);

        return cos;
    }

    void evaluate() override final {
        value = std::cos(a->value);
    }

    void derive() override final {
        a->partial -= partial * std::sin(a->value);
    }
};

struct Exp : Variable {

    Var a;

    Exp() {}

    static Var build(const Var& v) {

        shared_ptr<Exp> exp = make_shared<Exp>();

        exp->a = v;
        exp->name = "exp(" + v->name + ")";

        exp->parents.push_back(v);

        return exp;
    }

    void evaluate() override final {
        value = std::exp(a->value);
    }

    void derive() override final {
        a->partial += partial * value;
    }
};

struct Ln : Variable {

    Var a;

    Ln() {}

    static Var build(const Var& v) {

        shared_ptr<Ln> ln = make_shared<Ln>();

        ln->a = v;
        ln->name = "ln(" + v->name + ")";

        ln->parents.push_back(v);

        return ln;
    }

    void evaluate() override final {
        value = std::log(a->value);
    }

    void derive() override final {
        a->partial += partial / a->value;
    }
};

struct Div : Variable {

    Var a, b;

    Div() {}

    static Var build(const Var& v1, const Var& v2) {

        shared_ptr<Div> div = make_shared<Div>();

        div->a = v1;
        div->b = v2;
        div->name = "(" + v1->name + " / " + v2->name + ")";

        div->parents.push_back(v1);
        div->parents.push_back(v2);

        return div;
    }

    void evaluate() override final {
        value = a->value / b->value;
    }

    void derive() override final {
    	float inv = 1.0f / (b->value * b->value);
        a->partial += partial * b->value * inv;
        b->partial -= partial * a->value * inv;
    }
};


inline Var operator + (const Var& v1, const Var& v2) {
    return Add::build(v1, v2);
}

inline Var operator * (const Var& v1, const Var& v2) {
    return Mult::build(v1, v2);
}

inline Var operator / (const Var& v1, const Var& v2) {
    return Div::build(v1, v2);
}


inline Var operator + (const Var& v, float f) {
	return Add::build(v, Variable::build(f));
}

inline Var operator / (float f, const Var& v) {
	return Div::build(Variable::build(f), v);
}



int main() {

	cout << "\n";




    Var a1 = Variable::build(2.0f, "a1");
    Var a2 = Variable::build(6.0f, "a2");

    Var func = (a1 + a2) * a2;

    func->calculateDerivatives();

    cout << "Function: " << func->name << "\n";
    cout << "Value at (a1, a2) = (2, 6): " << func->value << "\n";
    cout << "Derivatives: \n - a1: " << a1->partial << "\n - a2: " << a2->partial << "\n\n";



    Var x = Variable::build(0.0f, "x");

    Var f = Sin::build(x) + Cos::build(x * x) + Exp::build(x * x * x) + Ln::build(x + 1.0f) + 1.0f / (x + 2.0f);

    f->calculateDerivatives();

    cout << "Function: " << f->name << "\n";
    cout << "Value at x = 1: " << f->value << "\n";
    cout << "Derivatives: \n - x: " << x->partial << "\n";

	return 0;
}
