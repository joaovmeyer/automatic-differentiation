#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <string>
#include <functional>


using namespace std;


int currentId = 0;

struct Node : enable_shared_from_this<Node> {
	vector<shared_ptr<Node>> childs;
	vector<shared_ptr<Node>> parents;
	string name;
	int id;

	Node(string n = "") : name(n) {
		id = currentId++;
	}

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
};

using NodePtr = std::shared_ptr<Node>;

struct Variable : Node {

    float value;
    float partial;

    Variable(float v = 0.0f, string n = "") {
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








// returns an order in wich if node i is a parent of node j,
// than node j will happen first than node i in the order.
// This is possible becaues the graph is acyclic
vector<NodePtr> topologicalSort(const NodePtr& root) {
	vector<NodePtr> ordering;
	unordered_set<int> visited;

	std::function<void(const NodePtr&)> addChildren = [&](const NodePtr& node) {
		if (visited.find(node->id) != visited.end()) {
			return;
		}

		visited.insert(node->id);
		for (size_t i = 0; i < node->childs.size(); ++i) {
			addChildren(node->childs[i]);
		}

		ordering.push_back(node);
	};

	addChildren(root);

	return ordering;
}


// this is similar to the first one, but it will not add nodes
// that are not influent to the given endNode. This is a bit more 
// costly to make but may avoid unnecessary calculations
vector<NodePtr> topologicalSort2(const NodePtr& endNode) {

	unordered_map<int, int> numChilds;
	vector<NodePtr> stack = { endNode };

	while (stack.size()) {
		NodePtr node = stack.back(); stack.pop_back();

		auto result = numChilds.insert({ node->id, 1 });
		if (!result.second) { // key was already in map
			++(result.first->second);
		} else {
			stack.insert(stack.end(), node->parents.begin(), node->parents.end());
		}
	}

	vector<NodePtr> ordering;

	stack = { endNode };
	while (stack.size()) {

		NodePtr node = stack.back(); stack.pop_back();
		ordering.push_back(node);

		for (size_t i = 0; i < node->parents.size(); ++i) {
			if (!(--numChilds[node->parents[i]->id])) {
				stack.push_back(node->parents[i]);
			}
		}
	}

	return ordering;
}






int main() {

    Var a1 = Variable::build(2.0f, "a1");
    Var a2 = Variable::build(6.0f, "a2");

    Var sum = Add::build(a1, a2);
    Var mult = Mult::build(sum, a2);

    vector<NodePtr> ordering = topologicalSort2(mult);

    for (int i = ordering.size() - 1; i >= 0; --i) {
        ordering[i]->evaluate();
	}

    mult->partial = 1.0;
    for (size_t i = 0; i < ordering.size(); ++i) {
        ordering[i]->derive();
	}

    cout << "Func: " << mult->name << "\n";
    cout << "Resultado: " << mult->value << "\n";
    cout << "Derivada: \n - a1: " << a1->partial << "\n - a2: " << a2->partial << "\n";
    cout << " - sum: " << sum->partial << "\n";

	return 0;
}
