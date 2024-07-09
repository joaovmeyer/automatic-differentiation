#ifndef NODE_HPP
#define NODE_HPP

#include <vector>
#include <memory>
#include <string>
#include <unordered_set>
#include <functional>


struct Node : std::enable_shared_from_this<Node> {
	std::vector<std::shared_ptr<Node>> parents;
	std::string name;

	Node(const std::string& n = "") : name(n) {

	}

	virtual inline void evaluate() = 0;
	virtual inline void derive() = 0;
	virtual inline void resetPartial() = 0;


	std::vector<std::shared_ptr<Node>> topologicalSort() {
		std::vector<std::shared_ptr<Node>> ordering;
		std::unordered_set<std::shared_ptr<Node>> visited;

		std::function<void(const std::shared_ptr<Node>&)> addChildren = [&](const std::shared_ptr<Node>& node) {
			// now we actually need "root" nodes because we need to reset their partials
			if (visited.find(node) != visited.end()/* || !node->parents.size()*/) {
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


#endif
