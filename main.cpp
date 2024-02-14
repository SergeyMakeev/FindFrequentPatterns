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
        std::vector<hash_t> hashes;              // linearized version of the above unordered_set
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

void generateDataSet(Dataset& dataset, size_t numSessions = 100, int numDifferentHashes = 100)
{
    srand(1379);
    dataset.clear();

    dataset.sessions.reserve(numSessions);
    for (size_t i = 0; i < numSessions; i++)
    {
        dataset.sessions.emplace_back();
        Dataset::Session& session = dataset.sessions.back();

        int numInstances = (rand() % 20) + 10;
        session.uniqueHashes.clear();
        for (size_t j = 0; j < (size_t)numInstances; j++)
        {
            // hash_t randomHash = rand() % 20000;
            hash_t randomHash = rand() % numDifferentHashes;
            session.uniqueHashes.insert(randomHash);
        }

        session.hashes.reserve(session.uniqueHashes.size());
        session.hashes.insert(session.hashes.end(), session.uniqueHashes.begin(), session.uniqueHashes.end());
        std::sort(session.hashes.begin(), session.hashes.end()); // to beautify print
    }
}

size_t getNumberOfSessionsThatContainsPattern(const Dataset& dataset, const Pattern& pattern)
{
    if (pattern.data.empty())
    {
        return 0;
    }

    size_t numSessionsThatContainsPattern = 0;

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

// threshold = 0.3f mean stop adding more hashes to the pattern (and return it) if less than 30% of the sessions have this pattern
Pattern findFrequentPattern(const Dataset& dataset, const Histogram& histogram, float threshold = 0.3f)
{
    //
    size_t totalSessionsCount = dataset.sessions.size();
    size_t numSessionsThreshold = size_t(0.5 + double(totalSessionsCount) * double(threshold));
    numSessionsThreshold = std::max(numSessionsThreshold, size_t(1));

    Pattern pattern;
    for (size_t step = 0; step < histogram.hashesSortedByFreq.size(); step++)
    {
        pattern.data.emplace_back(histogram.hashesSortedByFreq[step].first);
        size_t numSessions = getNumberOfSessionsThatContainsPattern(dataset, pattern);
        if (numSessions < numSessionsThreshold)
        {
            // remove the last hash from the pattern
            pattern.data.pop_back();
            break;
        }
    }

    std::sort(pattern.data.begin(), pattern.data.end()); // to beautify print
    return pattern;
}

void removePatternFromDataset(Dataset& dataset, const Pattern& pattern)
{
    //
    for (Dataset::Session& session : dataset.sessions)
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
            // remove pattern if found
            for (size_t i = 0; i < pattern.data.size(); i++)
            {
                session.uniqueHashes.erase(pattern.data[i]);
            }

            // update linearized (do we really need this?)
            session.hashes.clear();
            session.hashes.insert(session.hashes.end(), session.uniqueHashes.begin(), session.uniqueHashes.end());
            std::sort(session.hashes.begin(), session.hashes.end()); // to beautify print
        }
    }
}


void printSessionsThatContainsPattern(const Dataset& dataset, const Pattern& pattern)
{
    if (pattern.data.empty())
    {
        return;
    }

    int numMatchedSessions = 0;
    int sessionId = 0;
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
            numMatchedSessions++;
            printf("id[%d] = { ", sessionId);
            for (const hash_t& hash : session.hashes)
            {
                printf("%d ", int(hash));
            }

            printf("}\n");
        }

        sessionId++;
    }


    printf("Total: %d sessions, %3.2f %%\n", numMatchedSessions, 100.0 * double(numMatchedSessions) / double(dataset.sessions.size()));

}


void printDataset(const Dataset& dataset)
{
    //
    int sessionId = 0;
    for (const Dataset::Session& session : dataset.sessions)
    {
        printf("id[%d] = { ", sessionId);
        for (const hash_t& hash: session.hashes)
        {
            printf("%d ", int(hash));
        }

        printf("}\n");
        sessionId++;
    }
}

void printPattern(const Pattern& pattern)
{
    //
    for (size_t i = 0; i < pattern.data.size(); i++)
    {
        if (i>0)
        {
            printf(", ");
        }

        printf("%d", int(pattern.data[i]));
    }

    printf("\n");
}

int main()
{
    printf("Generate dataset\n");
    Dataset dataset;
    generateDataSet(dataset, 60, 10);
    printf("Done\n");

    Dataset datasetOriginal = dataset;

    printDataset(dataset);

    printf("Build histogram\n");
    Histogram histogram;
    buildHistogramAndFreq(dataset, histogram);
    printf("Done\n");

    // step #1
    // find frequent patterns
    printf("\n");
    printf("Freq patterns\n");
    for (int step = 0; step < 5; step++)
    {
        Pattern pattern = findFrequentPattern(dataset, histogram, 0.25f);
        printPattern(pattern);
        printSessionsThatContainsPattern(dataset, pattern);

        removePatternFromDataset(dataset, pattern);
        buildHistogramAndFreq(dataset, histogram);
        printf("\n");

        if (pattern.data.size() <= 2)
        {
            //this pattern is too smal (not interesting)
            //stop looking for more patterns
            break;
        }
    }

    // step #2
    // try to improve existing patterns (since we are removing parts of the dataset we can lose some information)

    printf("Existing\n");
    return 0;
}
