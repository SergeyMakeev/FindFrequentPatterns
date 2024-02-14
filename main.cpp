#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <assert.h>


using hash_t = uint64_t;

struct Dataset
{
	struct Session
	{
		std::vector<hash_t> hashes;
	};

	std::vector<Session> sessions;

	void clear()
	{
		sessions.clear();
	}
};


struct Scratch
{
	std::unordered_map<hash_t, size_t> hashToIndex;
	std::vector<hash_t> indexToHash;

	std::vector<uint64_t> histogram;
};


hash_t buildHistogramAndReturnMostFrequentHash(const Dataset& dataset, Scratch& scratch)
{
	const std::vector<Dataset::Session>& sessions = dataset.sessions;

	// find unique hashes
	std::unordered_map<hash_t, size_t>& hashToIndex = scratch.hashToIndex;
	hashToIndex.clear();
	for (const Dataset::Session& session : sessions)
	{
		for (const hash_t& hash : session.hashes)
		{
			hashToIndex.emplace(hash, 0);
		}
	}

	if (hashToIndex.size() < 2)
	{
		int yyy = 0;

	}
	assert(hashToIndex.size() > 2);

	// assign indices
	std::vector<hash_t>& indexToHash = scratch.indexToHash;
	indexToHash.clear();
	indexToHash.resize(hashToIndex.size());

	size_t index = 0;
	for (auto it = hashToIndex.begin(); it != hashToIndex.end(); ++it)
	{
		it->second = index;
		indexToHash[index] = it->first;
		index++;
	}

	// build histogram
	std::vector<uint64_t>& histogram = scratch.histogram;
	histogram.clear();
	histogram.resize(hashToIndex.size(), 0);

	for (const Dataset::Session& session : sessions)
	{
		for (const hash_t& hash : session.hashes)
		{
			auto it = hashToIndex.find(hash);
			assert(it != hashToIndex.end());

			size_t bucketIndex = it->second;
			histogram[bucketIndex]++;
		}
	}

	// find the largest bucket
	size_t maxBucketIndex = 0;
	uint64_t maxBucketValue = histogram[0];
	for (size_t bucketIndex = 1; bucketIndex < histogram.size(); bucketIndex++)
	{
		uint64_t currentBucketValue = histogram[bucketIndex];
		if (currentBucketValue > maxBucketValue)
		{
			maxBucketIndex = bucketIndex;
			maxBucketValue = currentBucketValue;
		}
	}

	return indexToHash[maxBucketIndex];
}


void generateDataSet(Dataset& dataset, size_t numSessions = 1000)
{
	srand(1379);

	dataset.clear();

	std::unordered_set<hash_t> uniqueHashes;

	dataset.sessions.reserve(numSessions);
	for (size_t i = 0; i < numSessions; i++)
	{
		dataset.sessions.emplace_back();
		Dataset::Session& session = dataset.sessions.back();

		int numInstances = (rand() % 5000) + 5000;
		uniqueHashes.clear();
		for (size_t j = 0; j < (size_t)numInstances; j++)
		{
			hash_t randomHash = rand() % 20000;
			uniqueHashes.insert(randomHash);
		}

		session.hashes.reserve(uniqueHashes.size());
		session.hashes.insert(session.hashes.end(), uniqueHashes.begin(), uniqueHashes.end());
	}
}


void splitDataset(Dataset& initialDataset, hash_t hashValue, Dataset& withHashValue)
{
	for (size_t s_id = 0; s_id < initialDataset.sessions.size(); s_id++)
	{
		Dataset::Session& session = initialDataset.sessions[s_id];
		bool hasValue = false;
		for (size_t h_id = 0; h_id < session.hashes.size(); h_id++)
		{
			const hash_t& hash = session.hashes[h_id];
			if (hash == hashValue)
			{
				hasValue = true;
				// remove hash
				session.hashes.erase(session.hashes.begin() + h_id);
				h_id--;
			}
		}

		if (hasValue)
		{
			withHashValue.sessions.emplace_back(std::move(session));
			initialDataset.sessions.erase(initialDataset.sessions.begin() + s_id);
			s_id--;
		}
	}
}


struct Tree
{
	struct Node
	{
		hash_t hash = hash_t(0);
		Dataset dataset;
		std::vector<std::unique_ptr<Node>> children;

		Node* addChild(hash_t hash)
		{
			std::unique_ptr<Node> node = std::make_unique<Node>();
			node->hash = hash;
			children.emplace_back(std::move(node));
			return children.back().get();
		}
	};

	std::unique_ptr<Node> root;

	Tree()
	{
		root = std::make_unique<Node>();
	}
};



void splitTreeNode(Tree::Node* node, Scratch& scratch, std::vector<Tree::Node*>& newNodes)
{
	size_t numSessions = node->dataset.sessions.size();
	size_t numSessionsThreshold = numSessions / 2;

	// build children nodes
	for (size_t rootIndex = 0; rootIndex < 10000; rootIndex++)
	{
		hash_t newNodeHash = buildHistogramAndReturnMostFrequentHash(node->dataset, scratch);
		Tree::Node* newNode = node->addChild(newNodeHash);
		newNodes.emplace_back(newNode);
		splitDataset(node->dataset, newNodeHash, newNode->dataset);

		// not much sessions left = not worth adding more children (they sorted by popularity)
		if (node->dataset.sessions.size() <= numSessionsThreshold)
		{
			break;
		}
	}
}



int main()
{
	printf("Generate dataset\n");
	printf("Build histogram\n");
	Scratch scratch;


	Tree tree;
	generateDataSet(tree.root->dataset, 10);


	std::vector<Tree::Node*> nodesToProcess;
	nodesToProcess.reserve(50000);

	nodesToProcess.emplace_back(tree.root.get());

	int numSplitNodes = 0;
	while (nodesToProcess.size() > 0)
	{
		if (numSplitNodes % 100)
		{
			printf("%d\n", numSplitNodes);
		}
		Tree::Node* node = nodesToProcess.back();
		nodesToProcess.pop_back();
		splitTreeNode(tree.root.get(), scratch, nodesToProcess);
		numSplitNodes++;
	}

	printf("Done\n");
	return 0;
}

