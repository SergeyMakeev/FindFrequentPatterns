#include <algorithm>
#include <assert.h>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using hash_t = uint64_t;

struct Dataset
{
    struct Session
    {
        std::unordered_set<hash_t> uniqueHashes; // we don't really need hash table here, sorted array will work fine
        std::vector<hash_t> hashes; // linearized version of the above unordered_set

    };

    std::vector<Session> sessions;

    void clear() { sessions.clear(); }
};

struct Histogram
{
    std::unordered_map<hash_t, size_t> hashToIndex;
    std::vector<hash_t> indexToHash;

    std::vector<std::pair<hash_t, uint64_t>> hashesSortedByFreq;

    std::vector<uint64_t> bins;
};

struct Pattern
{
    std::vector<hash_t> data;
};

void buildHistogramAndFreq(const Dataset& dataset, Histogram& histogram)
{
    const std::vector<Dataset::Session>& sessions = dataset.sessions;

    // find unique hashes
    std::unordered_map<hash_t, size_t>& hashToIndex = histogram.hashToIndex;
    hashToIndex.clear();
    for (const Dataset::Session& session : sessions)
    {
        for (const hash_t& hash : session.hashes)
        {
            hashToIndex.emplace(hash, 0);
        }
    }

    assert(hashToIndex.size() > 2);

    // assign indices
    std::vector<hash_t>& indexToHash = histogram.indexToHash;
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
    std::vector<uint64_t>& bins = histogram.bins;
    bins.clear();
    bins.resize(hashToIndex.size(), 0);

    for (const Dataset::Session& session : sessions)
    {
        for (const hash_t& hash : session.hashes)
        {
            auto it = hashToIndex.find(hash);
            assert(it != hashToIndex.end());

            size_t bucketIndex = it->second;
            bins[bucketIndex]++;
        }
    }

    // find the largest bucket
    size_t maxBucketIndex = 0;
    uint64_t maxBucketValue = bins[0];
    for (size_t bucketIndex = 1; bucketIndex < bins.size(); bucketIndex++)
    {
        uint64_t currentBucketValue = bins[bucketIndex];
        if (currentBucketValue > maxBucketValue)
        {
            maxBucketIndex = bucketIndex;
            maxBucketValue = currentBucketValue;
        }
    }

    std::vector<std::pair<hash_t, uint64_t>>& hashesSortedByFreq = histogram.hashesSortedByFreq;
    hashesSortedByFreq.clear();
    for (size_t bucketIndex = 0; bucketIndex < bins.size(); bucketIndex++)
    {
        hash_t hash = indexToHash[bucketIndex];
        uint64_t freq = bins[bucketIndex];
        hashesSortedByFreq.emplace_back(hash, freq);
    }

    std::sort(hashesSortedByFreq.begin(), hashesSortedByFreq.end(),
              [](const std::pair<hash_t, uint64_t>& lhs, const std::pair<hash_t, uint64_t>& rhs) { return lhs.second > rhs.second; });
}

void generateDataSet(Dataset& dataset, size_t numSessions = 1000)
{
    srand(1379);
    dataset.clear();

    dataset.sessions.reserve(numSessions);
    for (size_t i = 0; i < numSessions; i++)
    {
        dataset.sessions.emplace_back();
        Dataset::Session& session = dataset.sessions.back();

        int numInstances = (rand() % 5000) + 5000;
        session.uniqueHashes.clear();
        for (size_t j = 0; j < (size_t)numInstances; j++)
        {
            hash_t randomHash = rand() % 20000;
            session.uniqueHashes.insert(randomHash);
        }

        session.hashes.reserve(session.uniqueHashes.size());
        session.hashes.insert(session.hashes.end(), session.uniqueHashes.begin(), session.uniqueHashes.end());
    }
}

int getNumberOfSessionsThatContainsPattern(const Dataset& dataset, const Pattern& pattern)
{
    int numSessionsThatContainsPattern = 0;

    for (const Dataset::Session& session : dataset.sessions)
    {
        bool patternFound = true;
        for (size_t i = 0; i < pattern.data.size(); i++)
        {
            auto it = session.uniqueHashes.find(pattern.data[i]);
            if (it == session.uniqueHashes.end())
            {
                patternFound = false;
                break;
            }
        }

        if (patternFound)
        {
            numSessionsThatContainsPattern++;
        }
    }

    //
    return numSessionsThatContainsPattern;
}

int main()
{
    printf("Generate dataset\n");
    Dataset dataset;
    generateDataSet(dataset);
    printf("Done\n");

    printf("Build histogram\n");
    Histogram histogram;
    buildHistogramAndFreq(dataset, histogram);
    printf("Done\n");

    Pattern pattern;


    for (int step = 0; step < 100; step++)
    {
        pattern.data.emplace_back(histogram.hashesSortedByFreq[step].first);
        int numSessions = getNumberOfSessionsThatContainsPattern(dataset, pattern);
        printf("Pattern size: %d, Num sessions: %d\n", int(pattern.data.size()), numSessions);
    }

    printf("Existing\n");
    return 0;
}
