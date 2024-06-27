#include <iostream>
#include <vector>
#include <unordered_set>
#include <memory>
#include <string>
#include <functional>


using namespace std;



struct Node {
	vector<Node> childs;
	string name;
	int id;

	Node(vector<Node> c, string n, int i) : name(n), id(i) {
		childs.reserve(c.size());

		for (size_t i = 0; i < c.size(); ++i) {
			childs.push_back(c[i]);
		}
	}
};




vector<Node> topologicalSort(const Node& root) {
	vector<Node> ordering;
	unordered_set<int> visited;

	std::function<void(const Node&)> addChildren = [&](const Node& node) {
		if (visited.find(node.id) != visited.end()) {
			return;
		}

		visited.insert(node.id);
		for (size_t i = 0; i < node.childs.size(); ++i) {
			addChildren(node.childs[i]);
		}

		ordering.push_back(node);
	};

	addChildren(root);

	return ordering;
}


int main() {

	Node v5({}, "v5", 1);
	Node v4({ v5 }, "v4", 2);
	Node v1({ v4 }, "v1", 3);
	Node v2({ v4 }, "v2", 4);
	Node x({ v1, v2 }, "x", 5);

	vector<Node> ordering = topologicalSort(x);

	for (int i = ordering.size() - 1; i >= 0; --i) {
		cout << ordering[i].name << ", ";
	}


	return 0;
}
