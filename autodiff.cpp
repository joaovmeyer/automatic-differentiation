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
	int id;

	Node(string n, int i) : name(n), id(i) {
		
	}

	static shared_ptr<Node> createNode(const vector<shared_ptr<Node>>& c, string n, int i) {

		shared_ptr<Node> node = make_shared<Node>(n, i);

		node->childs.reserve(c.size());

		for (size_t i = 0; i < c.size(); ++i) {
			node->childs.push_back(c[i]);
			c[i]->parents.push_back(node);
		}

		return node;
	}
};

using NodePtr = std::shared_ptr<Node>;



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

	NodePtr v5 = Node::createNode({}, "v5", 1);
	NodePtr v4 = Node::createNode({ v5 }, "v4", 2);
	NodePtr v1 = Node::createNode({ v4 }, "v1", 3);
	NodePtr v2 = Node::createNode({ v4 }, "v2", 4);
	NodePtr x = Node::createNode({ v1, v2 }, "x", 5);


	vector<NodePtr> ordering = topologicalSort(x);

	for (int i = ordering.size() - 1; i >= 0; --i) {
		cout << ordering[i]->name << ", ";
	}


	return 0;
}
