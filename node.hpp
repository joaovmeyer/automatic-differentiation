#ifndef NODE_HPP
#define NODE_HPP

#include <vector>
#include <memory>
#include <string>
#include <unordered_set>
#include <functional>


// using the function's name can be helpfull for debugging, but sometimes it can break everything
// in the GRU implementation, the functions names were getting REALLY big, bigger than my RAM,
// so in this case it's best to avoid using the name at all
#define USE_NAME false


struct Node : std::enable_shared_from_this<Node> {
	std::vector<std::shared_ptr<Node>> parents;
	bool isTrainable;

	#if USE_NAME
		std::string name;
		Node(const std::string& n = "") : name(n) {}
	#else
		Node(const std::string& n = "") {}
	#endif

	enum NodeTypes {
		SCALAR,
		VECTOR,
		MATRIX
	};

	virtual inline void evaluate() = 0;
	virtual inline void derive() = 0;
	virtual inline void resetPartial() = 0;
	virtual inline NodeTypes getType() = 0;


	std::vector<std::shared_ptr<Node>> topologicalSort() {
		std::vector<std::shared_ptr<Node>> ordering;
		std::unordered_set<std::shared_ptr<Node>> visited;

		std::function<void(const std::shared_ptr<Node>&)> addChildren = [&](const std::shared_ptr<Node>& node) {
			// already handled this node
			if (visited.find(node) != visited.end()) {
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

	void eval() {
		std::vector<std::shared_ptr<Node>> ordering = topologicalSort();

		for (size_t i = 0; i < ordering.size(); ++i) {
			ordering[i]->evaluate();
		}
	}
};


#endif
