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

    float value;
    float partial;

	Node(float v = 0, float p = 0, string n = "") : value(v), partial(p), name(n) {
		id = currentId++;
	}

    virtual inline void evaluate() {};
    virtual inline void derive() {};


	static shared_ptr<Node> createNode(const vector<shared_ptr<Node>>& c, float v, float p, string n) {

		shared_ptr<Node> node = make_shared<Node>(v, p, n);

		node->childs.reserve(c.size());

		for (size_t i = 0; i < c.size(); ++i) {
			node->childs.push_back(c[i]);
			c[i]->parents.push_back(node);
		}

		return node;
	}

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

using Expression = Node;
/*
struct Expression : Node {
    float value;
    float partial;

    Expression(float v = 0, float p = 0) : value(v), partial(p) {}

    virtual inline void evaluate() = 0;
    virtual inline void derive(float seed = 1.0f) = 0;
};*/

using ExpPtr = std::shared_ptr<Expression>;

struct Variable : Expression {

    Variable(float v) {
        value = v;
    }

    void evaluate() {

    }

    void derive() {

    }
};

struct Add : Expression {

    ExpPtr a, b;

    Add() {}

    static NodePtr build(ExpPtr a1, ExpPtr a2) {

        shared_ptr<Add> add = make_shared<Add>();
        NodePtr addNode = dynamic_pointer_cast<Node>(add);

        add->a = a1;
        add->b = a2;

        add->a->addChildren(addNode);
        add->b->addChildren(addNode);

        return addNode;
    }

    void evaluate() {
        value = a->value + b->value;
    }

    void derive() {
        a->partial += partial;
        b->partial += partial;
    }
};

struct Mult : Expression {

    ExpPtr a, b;

    Mult() {}

    static NodePtr build(ExpPtr a1, ExpPtr a2) {

        shared_ptr<Mult> add = make_shared<Mult>();
        NodePtr addNode = dynamic_pointer_cast<Node>(add);

        add->a = a1;
        add->b = a2;

        add->a->addChildren(addNode);
        add->b->addChildren(addNode);

        return addNode;
    }

    void evaluate() {
        value = a->value * b->value;
    }

    void derive() {
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
    
//    (x + y) * y at (2, 6)

    ExpPtr a1 = make_shared<Expression>(2.0f);
    ExpPtr a2 = make_shared<Expression>(6.0f);

    ExpPtr sum = Add::build(a1, a2);
    ExpPtr mult = Mult::build(sum, a2);

    vector<NodePtr> ordering = topologicalSort2(mult);

    for (int i = ordering.size() - 1; i >= 0; --i) {
        cout << ordering[i]->id << ", ";
        ordering[i]->evaluate();
	}

    mult->partial = 1.0;
    for (size_t i = 0; i < ordering.size(); ++i) {
        ordering[i]->derive();
	}

    cout << "Resultado: " << mult->value << "\n";
    cout << "Derivada: \n - a1: " << a1->partial << "\n - a2: " << a2->partial << "\n";
    cout << " - sum: " << sum->partial << "\n";




	return 0;
}
