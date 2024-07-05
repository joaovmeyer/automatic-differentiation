#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <string>
#include <functional>


using namespace std;



struct Node : enable_shared_from_this<Node> {
	vector<shared_ptr<Node>> childs;
	vector<shared_ptr<Node>> parents;
	string name;
	size_t id;

	Node(string n = "") : name(n), id(Node::currentId++) {

	}

	static size_t currentId;

    virtual inline void evaluate() = 0;
    virtual inline void derive() = 0;


    void addChildren(const vector<shared_ptr<Node>>& c) {
        childs.insert(childs.end(), c.begin(), c.end());

        for (size_t i = 0; i < c.size(); ++i) {
			c[i]->parents.push_back(shared_from_this());
		}
    }

    void addChildren(const shared_ptr<Node>& c) {
        childs.push_back(c);
        c->parents.push_back(shared_from_this());
    }


    vector<std::shared_ptr<Node>> topologicalSort() {
    	vector<std::shared_ptr<Node>> ordering;
		unordered_set<int> visited;

		std::function<void(const std::shared_ptr<Node>&)> addChildren = [&](const std::shared_ptr<Node>& node) {
			// if the node has no parents, we shouldn't need to either evaluate nor derive it
			if (visited.find(node->id) != visited.end() || !node->parents.size()) {
				return;
			}

			visited.insert(node->id);
			for (size_t i = 0; i < node->parents.size(); ++i) {
				addChildren(node->parents[i]);
			}

			ordering.push_back(node);
		};

		addChildren(shared_from_this());

		return ordering;
	}
};

size_t Node::currentId = 0;

struct Variable : Node {

    float value;
    float partial;

    Variable(float v = 0.0f, string n = "") : partial(0.0f) {
        value = v;
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

    static Var build(Var a1, Var a2) {

        shared_ptr<Add> add = make_shared<Add>();

        add->a = a1;
        add->b = a2;
        add->name = "(" + a1->name + " + " + a2->name + ")";

        add->a->addChildren(add);
        add->b->addChildren(add);

        return add;
    }

    void evaluate() override {
        value = a->value + b->value;
    }

    void derive() override {
        a->partial += partial;
        b->partial += partial;
    }
};

struct Mult : Variable {

    Var a, b;

    Mult() {}

    static Var build(Var a1, Var a2) {

        shared_ptr<Mult> mult = make_shared<Mult>();

        mult->a = a1;
        mult->b = a2;
        mult->name = "(" + a1->name + " * " + a2->name + ")";

        mult->a->addChildren(mult);
        mult->b->addChildren(mult);

        return mult;
    }

    void evaluate() override {
        value = a->value * b->value;
    }

    void derive() override {
        a->partial += partial * b->value;
        b->partial += partial * a->value;
    }
};


Var operator + (const Var& v1, const Var& v2) {
    return Add::build(v1, v2);
}

Var operator * (const Var& v1, const Var& v2) {
    return Mult::build(v1, v2);
}






int main() {

    Var a1 = Variable::build(2.0f, "a1");
    Var a2 = Variable::build(6.0f, "a2");

    Var func = (a1 + a2) * a2;

    func->calculateDerivatives();

    cout << "Function: " << func->name << "\n";
    cout << "Value at (a1, a2) = (2, 6): " << func->value << "\n";
    cout << "Derivatives: \n - a1: " << a1->partial << "\n - a2: " << a2->partial << "\n";

	return 0;
}
