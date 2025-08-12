#ifndef NODE_HPP
#define NODE_HPP

#include <vector>
#include <memory>
#include <string>
#include <unordered_set>
#include <functional>
#include <tuple>
#include <omp.h>



#ifndef NUM_TYPE
#define NUM_TYPE NUM_TYPE
#endif


// using the function's name can be helpfull for debugging, but sometimes it can break everything
// in the GRU implementation, the functions names were getting REALLY big, bigger than my RAM,
// so in this case it's best to avoid using the name at all
#ifndef USE_NAME
#define USE_NAME false
#endif


struct Node : std::enable_shared_from_this<Node> {
	std::vector<std::shared_ptr<Node>> parents;
	bool isTrainable;
	bool isSlowOperation = false;

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
	virtual inline void resetPartial(NUM_TYPE defaultValue = 0.0f) = 0;
	virtual inline NodeTypes getType() = 0;
	virtual inline void updateGradientFunction() = 0; // similar to derive but to the function, not partial
	virtual inline void resetGradientFunction(NUM_TYPE defaultValue = 0.0f) = 0; // similar to resetPartial, ...


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

	void calculateDerivatives() {
		std::vector<std::shared_ptr<Node>> ordering = topologicalSort();

		for (size_t i = 0; i < ordering.size(); ++i) {
			ordering[i]->evaluate();
			ordering[i]->resetPartial();
		}

		// dx/dx is 1 for whatever x
		resetPartial(1.0f);
		for (size_t i = ordering.size(); i > 0; --i) {
			ordering[i - 1]->derive();
		}
	}


	void calculateGradientFunctions() {
		std::vector<std::shared_ptr<Node>> ordering = topologicalSort();

		for (size_t i = 0; i < ordering.size(); ++i) {
			ordering[i]->resetGradientFunction();
		}

		// dx/dx is 1 for whatever x
		resetGradientFunction(1.0f);
		for (size_t i = ordering.size(); i > 0; --i) {
			ordering[i - 1]->updateGradientFunction();
		}
	}


	using NodeMat = std::vector<std::vector<std::shared_ptr<Node>>>;
	

	// similar to topologicalSort, but separated in layers, where each layer can be processed in parallel
	// this is a very crude implementation, and probably I should do some tweaks to maximize the size
	// of each slow layer, at least it makes sense to me that packing them together will be faster

	// I also tried to separate them in slow layers and fast layers, because parallelizing the fast operations
	// is not worth it. Probably there's a better way to do this, but that's for the future.
	std::tuple<NodeMat, NodeMat> layeredTopologicalSort(bool excludeFirstGeneration = false) {
		NodeMat layersSlow;
		NodeMat layersFast;
		std::unordered_set<std::shared_ptr<Node>> visited;
		std::unordered_map<std::shared_ptr<Node>, int> layerNum;

		std::function<void(const std::shared_ptr<Node>&)> addChildren = [&](const std::shared_ptr<Node>& node) {
			// already handled this node
			if (visited.find(node) != visited.end() || (excludeFirstGeneration && !node->parents.size())) {
				return;
			}

			visited.insert(node);

			int maxParentLayer = -1;
			for (size_t i = 0; i < node->parents.size(); ++i) {
				addChildren(node->parents[i]);
				maxParentLayer = std::max(maxParentLayer, layerNum[node->parents[i]]);
			}

			int nodeLayer = maxParentLayer + 1;
			layerNum[node] = nodeLayer;

			// resize both layers cause they should have the same size
			layersSlow.resize(std::max(layersSlow.size(), (size_t) nodeLayer + 1));
			layersFast.resize(std::max(layersFast.size(), (size_t) nodeLayer + 1));

			if (node->isSlowOperation) {
				layersSlow[nodeLayer].push_back(node);
			} else {
				layersFast[nodeLayer].push_back(node);
			}
		};

		addChildren(shared_from_this());

		return { layersFast, layersSlow };
	}

	// now this is generally not worth it, but if a given function has some heavy calculations that can be done in parallel,
	// there's a chance the layered topological sort will arrange them together, and you might get some pretty good speed up.
	// As an example, in an LSTM cell, there are 8 matrix by vector multiplications, and it just happens that it's parallel
	// version will separate them in two groups of 4, and I tested it with a big LSTM layer (512 neurons) and I got a ~1.5x
	// speed up over the non parallel version (execution time went from ~67 seconds to ~45 secunds)
	void calculateDerivativesParallel() {

		auto [layersFast, layersSlow] = layeredTopologicalSort();

		for (size_t i = 0; i < layersSlow.size(); ++i) {

			// for slow layers, try to parallelize what we can
			#pragma omp parallel for schedule(dynamic) if(layersSlow[i].size() > 1)
			for (size_t j = 0; j < layersSlow[i].size(); ++j) {
				layersSlow[i][j]->evaluate();
				layersSlow[i][j]->resetPartial();
			}


			// for fast layers, the overhead of parallelizing would actually cause a performance decrease
			for (size_t j = 0; j < layersFast[i].size(); ++j) {
				layersFast[i][j]->evaluate();
				layersFast[i][j]->resetPartial();
			}
		}

		// dx/dx is 1 for whatever x
		resetPartial(1.0f);

		for (size_t i = layersSlow.size(); i > 0; --i) {

			#pragma omp parallel for schedule(dynamic) if(layersSlow[i - 1].size() > 1)
			for (size_t j = 0; j < layersSlow[i - 1].size(); ++j) {
				layersSlow[i - 1][j]->derive();
			}


			for (size_t j = 0; j < layersFast[i - 1].size(); ++j) {
				layersFast[i - 1][j]->derive();
			}
		}
	}












	void eval() {
		std::vector<std::shared_ptr<Node>> ordering = topologicalSort();

		for (size_t i = 0; i < ordering.size(); ++i) {
			ordering[i]->evaluate();
		}
	}
};


#endif
