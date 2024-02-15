#include <algorithm>
#include <assert.h>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define REMOVE_EXISTING_PATTERN_FROM_DATA

using hash_t = uint64_t;

struct Dataset
{
    struct Session
    {
        std::unordered_set<hash_t> uniqueHashes; // we don't really need hash table here, sorted array will work fine
        std::vector<hash_t> hashes;              // linearized version of the above unordered_set

        Session() = default;
        Session(std::initializer_list<hash_t> list)
        {
            uniqueHashes = list;
            hashes.insert(hashes.end(), uniqueHashes.begin(), uniqueHashes.end());
            std::sort(hashes.begin(), hashes.end()); // to beautify print
        }
    };

    std::vector<Session> sessions;

    void clear() { sessions.clear(); }
};

struct Histogram
{
    std::unordered_map<hash_t, size_t> _hashToIndex;
    std::vector<hash_t> _indexToHash;

    std::vector<std::pair<hash_t, uint64_t>> hashesSortedByFreq;

    std::vector<uint64_t> bins;

    void trim_left()
    {
        bins.erase(bins.begin());
        hash_t mostPopularHash = hashesSortedByFreq.front().first;
        hashesSortedByFreq.erase(hashesSortedByFreq.begin());
    }
};

struct Pattern
{
    std::vector<hash_t> data;
};

bool buildHistogramAndFreq(const Dataset& dataset, Histogram& histogram)
{
    const std::vector<Dataset::Session>& sessions = dataset.sessions;

    // find unique hashes
    std::unordered_map<hash_t, size_t>& hashToIndex = histogram._hashToIndex;
    hashToIndex.clear();
    for (const Dataset::Session& session : sessions)
    {
        for (const hash_t& hash : session.hashes)
        {
            hashToIndex.emplace(hash, 0);
        }
    }

    if (hashToIndex.size() <= 2)
    {
        histogram.bins.clear();
        histogram.hashesSortedByFreq.clear();
        histogram._hashToIndex.clear();
        histogram._indexToHash.clear();
        return false;
    }
    assert(hashToIndex.size() > 2);

    // assign indices
    std::vector<hash_t>& indexToHash = histogram._indexToHash;
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

    histogram._indexToHash.clear();
    histogram._hashToIndex.clear();
    return true;
}

void generateToyDataSet(Dataset& dataset)
{
    /*
    dataset.clear();
    dataset.sessions.emplace_back(Dataset::Session({1, 2, 3, 4, 5, 55}));
    dataset.sessions.emplace_back(Dataset::Session({2, 3, 4, 5, 55, 66}));
    dataset.sessions.emplace_back(Dataset::Session({7, 2, 3, 4, 66, 77}));
    dataset.sessions.emplace_back(Dataset::Session({1, 2, 7, 9, 10, 55}));
    dataset.sessions.emplace_back(Dataset::Session({2, 3, 7, 55}));
    */

    // clang-format off
    dataset.clear();
    dataset.sessions.emplace_back(Dataset::Session({1, 2, 3, 4, 5,    7, 8, 9}));
    dataset.sessions.emplace_back(Dataset::Session({1, 2, 3, 4, 5,    7, 8, 9}));
    dataset.sessions.emplace_back(Dataset::Session({1, 2, 3, 4, 5            }));
    dataset.sessions.emplace_back(Dataset::Session({1, 2, 3, 4, 5            }));
    dataset.sessions.emplace_back(Dataset::Session({1, 2, 3, 4, 5            }));
    dataset.sessions.emplace_back(Dataset::Session({1, 2, 3, 4, 5            }));
    dataset.sessions.emplace_back(Dataset::Session({   2, 3, 4, 5, 6, 7, 8, 9}));
    dataset.sessions.emplace_back(Dataset::Session({   2, 3, 4, 5, 6, 7, 8, 9}));
    dataset.sessions.emplace_back(Dataset::Session({   2, 3, 4, 5, 6, 7, 8, 9}));
    dataset.sessions.emplace_back(Dataset::Session({   2, 3, 4, 5, 6, 7, 8, 9}));
    dataset.sessions.emplace_back(Dataset::Session({   2, 3, 4, 5, 6, 7, 8, 9}));
    dataset.sessions.emplace_back(Dataset::Session({   2, 3, 4, 5, 6, 7, 8, 9}));
    dataset.sessions.emplace_back(Dataset::Session({1, 2,    4, 5, 6         }));
    dataset.sessions.emplace_back(Dataset::Session({1, 2,    4, 5, 6         }));
    dataset.sessions.emplace_back(Dataset::Session({1, 2,    4, 5, 6         }));
    dataset.sessions.emplace_back(Dataset::Session({1, 2,    4, 5, 6         }));
    dataset.sessions.emplace_back(Dataset::Session({1, 2,    4, 5, 6         }));
    dataset.sessions.emplace_back(Dataset::Session({1, 2,    4, 5, 6         }));
    dataset.sessions.emplace_back(Dataset::Session({1,       4, 5, 6         }));
    dataset.sessions.emplace_back(Dataset::Session({1,       4, 5, 6         }));
    dataset.sessions.emplace_back(Dataset::Session({1,       4, 5, 6         }));
    dataset.sessions.emplace_back(Dataset::Session({1,       4, 5, 6         }));
    // clang-format on


    /*
        Patterns in the above dataset (generated using brute force algorithm):

        4, 5
        Total: 22 of 22 sessions, 100.00 %
        2, 4
        Total: 18 of 22 sessions, 81.82 %
        2, 5
        Total: 18 of 22 sessions, 81.82 %

        2, 4, 5
        Total: 18 of 22 sessions, 81.82 %

        1, 2, 4, 5
        Total: 12 of 22 sessions, 54.55 %
        2, 3, 4, 5
        Total: 12 of 22 sessions, 54.55 %
        2, 4, 5, 6
        Total: 12 of 22 sessions, 54.55 %

        3, 4, 5, 7, 9
        Total: 8 of 22 sessions, 36.36 %
        3, 4, 7, 8, 9
        Total: 8 of 22 sessions, 36.36 %
        3, 4, 5, 7, 8
        Total: 8 of 22 sessions, 36.36 %
        2, 3, 4, 7, 8
        Total: 8 of 22 sessions, 36.36 %
        2, 3, 7, 8, 9
        Total: 8 of 22 sessions, 36.36 %

        2, 3, 4, 5, 7, 8
        Total: 8 of 22 sessions, 36.36 %
        2, 3, 4, 7, 8, 9
        Total: 8 of 22 sessions, 36.36 %
        2, 3, 5, 7, 8, 9
        Total: 8 of 22 sessions, 36.36 %
        2, 4, 5, 7, 8, 9
        Total: 8 of 22 sessions, 36.36 %

        2, 3, 4, 5, 7, 8, 9
        Total: 8 of 22 sessions, 36.36 %

        2, 3, 4, 5, 6, 7, 8, 9
        Total: 6 of 22 sessions, 27.27 %
    */
}



void generateRandomDataSet(Dataset& dataset, size_t numSessions = 100, int numDifferentHashes = 100)
{
    srand(1379);
    dataset.clear();

    dataset.sessions.reserve(numSessions);
    for (size_t i = 0; i < numSessions; i++)
    {
        Dataset::Session& session = dataset.sessions.emplace_back();

        int numInstances = (rand() % 200) + 200;
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
    numSessionsThreshold = std::min(numSessionsThreshold, totalSessionsCount);

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

std::vector<size_t> getSessionsThatContainsPattern(const Dataset& dataset, const Pattern& pattern)
{
    std::vector<size_t> res;

    if (pattern.data.empty())
    {
        return res;
    }

    size_t sessionId = 0;
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
            res.emplace_back(sessionId);
        }
        sessionId++;
    }
    return res;
}

void printSessionsThatContainsPattern(const Dataset& dataset, const Pattern& pattern, bool detailed = true)
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

            if (detailed)
            {
                printf("id[%d] = { ", sessionId);
                for (const hash_t& hash : session.hashes)
                {
                    printf("%d ", int(hash));
                }

                printf("}\n");
            }
        }

        sessionId++;
    }

    printf("Total: %d of %d sessions, %3.2f %%\n", numMatchedSessions, int(dataset.sessions.size()),
           100.0 * double(numMatchedSessions) / double(dataset.sessions.size()));
}

void printDataset(const Dataset& dataset)
{
    //
    int sessionId = 0;
    for (const Dataset::Session& session : dataset.sessions)
    {
        printf("id[%d] = { ", sessionId);
        for (const hash_t& hash : session.hashes)
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
        if (i > 0)
        {
            printf(", ");
        }

        printf("%d", int(pattern.data[i]));
    }

    printf("\n");
}

struct Solution
{
    Pattern pattern;
    Pattern patternImproved;

    // matched session ids
    std::vector<size_t> sessionIds;
};

int main()
{
    printf("Generate dataset\n");
    Dataset dataset;
    // generateRandomDataSet(dataset, 60000, 75);
    generateToyDataSet(dataset);
    printf("Done\n");

    Dataset datasetOriginal = dataset;

    // printDataset(dataset);

    printf("Build histogram\n");
    Histogram histogram;
    buildHistogramAndFreq(dataset, histogram);
    printf("Done\n");

    // step #1
    // find frequent patterns
    printf("\n");
    printf("Freq patterns\n");

    std::vector<Solution> solutions;

    int kMaxNumberOfPatterns = 40;
    for (int step = 0; step < kMaxNumberOfPatterns; step++)
    {
        Solution& s = solutions.emplace_back();
        s.pattern = findFrequentPattern(dataset, histogram, 0.2f);
        s.sessionIds = getSessionsThatContainsPattern(dataset, s.pattern);

#ifdef REMOVE_EXISTING_PATTERN_FROM_DATA
        // remove frequent pattern from data
        removePatternFromDataset(dataset, s.pattern);
        if (!buildHistogramAndFreq(dataset, histogram))
        {
            // all sessions are now empty
            printf("Solved. All sessions are now empty\n");
            break;
        }
        // printf("\n");
#else
        // remove most popular item
        histogram.trim_left();
#endif

        if (s.pattern.data.size() <= 2)
        {
            // this pattern is too smal (no longer interesting)
            // stop looking for more patterns
            printf("Stop. The resulting pattern is too short (%d in %d sessions). No longer interesting in solving\n",
                   int(s.pattern.data.size()), int(s.sessionIds.size()));
            solutions.pop_back();
            break;
        }
    }

    // step #2
    // try to improve existing patterns (since we are removing parts of the dataset we can lose some information)
    for (size_t si = 0; si < solutions.size(); si++)
    {
        Dataset tempDataset;
        // copy sessions from original dataset to temp dataset
        Solution& s = solutions[si];
        for (size_t i = 0; i < s.sessionIds.size(); i++)
        {
            size_t sessionId = s.sessionIds[i];
            tempDataset.sessions.push_back(datasetOriginal.sessions[sessionId]);
        }

        buildHistogramAndFreq(tempDataset, histogram);

        s.patternImproved = findFrequentPattern(dataset, histogram, 1.0f);

        if (s.patternImproved.data.size() > s.pattern.data.size())
        {
            printf("-> Improved pattern:\n");
            printPattern(s.patternImproved);
            printSessionsThatContainsPattern(datasetOriginal, s.patternImproved, false);
        }
        else
        {
            printf("-> Pattern:\n");
            printPattern(s.pattern);
            printSessionsThatContainsPattern(datasetOriginal, s.pattern, false);
        }
    }

    printf("Existing\n");
    return 0;
}
