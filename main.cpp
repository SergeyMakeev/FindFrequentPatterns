//
// Highly Scalable SIMD approach for Efficiently Mining Maximal Frequent Itemsets
//
#include <algorithm>
#include <assert.h>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using id_t = uint64_t;

struct Dataset
{
    struct Transaction
    {
        std::unordered_set<id_t> uniqueIds;
        Transaction() = default;
        Transaction(std::initializer_list<id_t> list) { uniqueIds = list; }
    };

    std::vector<Transaction> transactions;

    void clear() { transactions.clear(); }
};

struct Bitset
{
    using Bitword = uint64_t;
    static constexpr size_t kBitwordSizeInBits = sizeof(Bitword) * 8;

    inline size_t roundupToBitwords(size_t numBits) { return (numBits + kBitwordSizeInBits - 1) / kBitwordSizeInBits; }

    std::vector<Bitword> bitwords;

    Bitset() = delete;

    Bitset(size_t numBits)
    {
        size_t numBitwords = roundupToBitwords(numBits);
        bitwords.resize(numBitwords, Bitword(0));
    }

    size_t size() const { return bitwords.size() * kBitwordSizeInBits; }

    void set(size_t bitIndex)
    {
        size_t bitwordIndex = bitIndex / kBitwordSizeInBits;
        assert(bitwordIndex < bitwords.size());
        Bitword& bitword = bitwords[bitwordIndex];
        Bitword mask = (Bitword(1) << (Bitword(bitIndex) % Bitword(kBitwordSizeInBits)));
        bitword |= mask;
    }

    void reset(size_t bitIndex)
    {
        size_t bitwordIndex = bitIndex / kBitwordSizeInBits;
        assert(bitwordIndex < bitwords.size());
        Bitword& bitword = bitwords[bitwordIndex];
        Bitword mask = (Bitword(1) << (Bitword(bitIndex) % Bitword(kBitwordSizeInBits)));
        bitword &= ~mask;
    }

    void toggle(size_t bitIndex)
    {
        size_t bitwordIndex = bitIndex / kBitwordSizeInBits;
        assert(bitwordIndex < bitwords.size());
        Bitword& bitword = bitwords[bitwordIndex];
        Bitword mask = (Bitword(1) << (Bitword(bitIndex) % Bitword(kBitwordSizeInBits)));
        bitword ^= mask;
    }

    bool get(size_t bitIndex) const
    {
        size_t bitwordIndex = bitIndex / kBitwordSizeInBits;
        assert(bitwordIndex < bitwords.size());
        const Bitword& bitword = bitwords[bitwordIndex];
        Bitword mask = (Bitword(1) << (Bitword(bitIndex) % Bitword(kBitwordSizeInBits)));
        return ((bitword & mask) != 0);
    }

    inline size_t count() const
    {
        size_t numWords = bitwords.size();
        size_t numEnabledBits = 0;
        for (size_t i = 0; i < numWords; i++)
        {
            numEnabledBits += size_t(__popcnt64(bitwords[i]));
        }
        return numEnabledBits;
    }

    inline static Bitset match(const Bitset& a, const Bitset& b)
    {
        assert(a.bitwords.size() == b.bitwords.size());

        Bitset res(a.bitwords.size() * Bitset::kBitwordSizeInBits);
        size_t numWords = a.bitwords.size();
        for (size_t i = 0; i < numWords; i++)
        {
            res.bitwords[i] = a.bitwords[i] & b.bitwords[i];
        }
        return res;
    }

    inline static size_t match_count(const Bitset& a, const Bitset& b)
    {
        assert(a.bitwords.size() == b.bitwords.size());

        size_t numMatchedBits = 0;
        size_t numWords = a.bitwords.size();
        for (size_t i = 0; i < numWords; i++)
        {
            numMatchedBits += size_t(__popcnt64(a.bitwords[i] & b.bitwords[i]));
        }
        return numMatchedBits;
    }
};

bool operator==(const Bitset& lhs, const Bitset& rhs) { return lhs.bitwords == rhs.bitwords; }

namespace std
{
template <> struct hash<Bitset>
{
    std::size_t operator()(const Bitset& k) const
    {
        std::size_t seed = 0;
        for (Bitset::Bitword i : k.bitwords)
        {
            seed ^= std::hash<Bitset::Bitword>()(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};
} // namespace std

void generateToyDataSet(Dataset& dataset)
{
    // clang-format off
    dataset.clear();
    dataset.transactions.emplace_back(Dataset::Transaction({1, 2, 3, 4, 5,    7, 8, 9}));
    dataset.transactions.emplace_back(Dataset::Transaction({1, 2, 3, 4, 5,    7, 8, 9}));
    dataset.transactions.emplace_back(Dataset::Transaction({1, 2, 3, 4, 5            }));
    dataset.transactions.emplace_back(Dataset::Transaction({1, 2, 3, 4, 5            }));
    dataset.transactions.emplace_back(Dataset::Transaction({1, 2, 3, 4, 5            }));
    dataset.transactions.emplace_back(Dataset::Transaction({1, 2, 3, 4, 5            }));
    dataset.transactions.emplace_back(Dataset::Transaction({   2, 3, 4, 5, 6, 7, 8, 9}));
    dataset.transactions.emplace_back(Dataset::Transaction({   2, 3, 4, 5, 6, 7, 8, 9}));
    dataset.transactions.emplace_back(Dataset::Transaction({   2, 3, 4, 5, 6, 7, 8, 9}));
    dataset.transactions.emplace_back(Dataset::Transaction({   2, 3, 4, 5, 6, 7, 8, 9}));
    dataset.transactions.emplace_back(Dataset::Transaction({   2, 3, 4, 5, 6, 7, 8, 9}));
    dataset.transactions.emplace_back(Dataset::Transaction({   2, 3, 4, 5, 6, 7, 8, 9}));
    dataset.transactions.emplace_back(Dataset::Transaction({1, 2,    4, 5, 6         }));
    dataset.transactions.emplace_back(Dataset::Transaction({1, 2,    4, 5, 6         }));
    dataset.transactions.emplace_back(Dataset::Transaction({1, 2,    4, 5, 6         }));
    dataset.transactions.emplace_back(Dataset::Transaction({1, 2,    4, 5, 6         }));
    dataset.transactions.emplace_back(Dataset::Transaction({1, 2,    4, 5, 6         }));
    dataset.transactions.emplace_back(Dataset::Transaction({1, 2,    4, 5, 6         }));
    dataset.transactions.emplace_back(Dataset::Transaction({1,       4, 5, 6         }));
    dataset.transactions.emplace_back(Dataset::Transaction({1,       4, 5, 6         }));
    dataset.transactions.emplace_back(Dataset::Transaction({1,       4, 5, 6         }));
    dataset.transactions.emplace_back(Dataset::Transaction({1,       4, 5, 6         }));
    // clang-format on
}

// note: the resulting dataset can be smaller than minElements because we generate IDs randomly
void generateRandomDataSet(Dataset& dataset, size_t numTransactions = 100, int minElements = 200, int maxElements = 400,
                           int numDifferentIds = 100)
{
    assert(maxElements > minElements);
    srand(1379);
    dataset.clear();
    dataset.transactions.reserve(numTransactions);

    int numElements = (maxElements - minElements);

    for (size_t i = 0; i < numTransactions; i++)
    {
        Dataset::Transaction& transaction = dataset.transactions.emplace_back();

        int numInstances = (rand() % numElements) + minElements;
        transaction.uniqueIds.clear();
        for (size_t j = 0; j < (size_t)numInstances; j++)
        {
            id_t randomId = rand() % numDifferentIds;
            transaction.uniqueIds.insert(randomId);
        }
    }
}

struct Mapping
{
    std::unordered_map<id_t, size_t> idToIndex;
    std::vector<id_t> indexToId;
};

Mapping getDatasetMapping(const Dataset& dataset)
{
    Mapping mapping;
    for (const Dataset::Transaction& transaction : dataset.transactions)
    {
        for (auto it = transaction.uniqueIds.begin(); it != transaction.uniqueIds.end(); ++it)
        {
            mapping.idToIndex.try_emplace(*it, 0);
        }
    }

    mapping.indexToId.resize(mapping.idToIndex.size(), 0);

    size_t index = 0;
    for (auto it = mapping.idToIndex.begin(); it != mapping.idToIndex.end(); ++it)
    {
        mapping.indexToId[index] = it->first;
        it->second = index;
        index++;
    }

    return mapping;
}

std::vector<id_t> getPattern(const Bitset& pattern, const Mapping& mapping)
{
    std::vector<id_t> res;
    for (size_t index = 0; index < pattern.size(); index++)
    {
        if (pattern.get(index))
        {
            assert(index < mapping.indexToId.size());
            id_t id = mapping.indexToId[index];
            res.emplace_back(id);
        }
    }
    return res;
}

std::vector<size_t> getTransactionsThatMatchPattern(const Dataset& dataset, const std::vector<id_t>& pattern)
{
    std::vector<size_t> res;

    if (pattern.empty())
    {
        return res;
    }

    size_t sessionId = 0;
    for (const Dataset::Transaction& transaction : dataset.transactions)
    {
        bool patternFound = true;
        for (size_t i = 0; i < pattern.size(); i++)
        {
            auto it = transaction.uniqueIds.find(pattern[i]);
            if (it == transaction.uniqueIds.end())
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

size_t _getNumTransactionsThatMatchPattern(const std::vector<Bitset>& bitsetTransactions, const Bitset& pattern)
{
    size_t numMatched = 0;
    size_t numEnabledBits = pattern.count();
    for (size_t i = 0; i < bitsetTransactions.size(); i++)
    {
        const Bitset& a = bitsetTransactions[i];
        if (Bitset::match_count(a, pattern) == numEnabledBits)
        {
            numMatched++;
        }
    }

    return numMatched;
}

void printIds(std::vector<id_t>& ids)
{
    // need to sort to beautify output
    std::sort(ids.begin(), ids.end());
    for (const id_t& id : ids)
    {
        printf("%d ", uint32_t(id));
    }
}

void printDataset(const Dataset& dataset)
{
    printf("Dataset\n");
    int transactionId = 0;
    for (const Dataset::Transaction& transaction : dataset.transactions)
    {
        printf("%d: ", transactionId);
        printIds(std::vector<id_t>(transaction.uniqueIds.begin(), transaction.uniqueIds.end()));
        printf("\n");
        transactionId++;
    }
}

void printPattern(std::vector<id_t>& pattern, const std::vector<size_t> transactionsList, size_t numTransactionsTotal)
{
    //
    printf("%3.2f%% ; %d / %d ; ", 100.0 * double(transactionsList.size()) / double(numTransactionsTotal),
           uint32_t(transactionsList.size()), uint32_t(numTransactionsTotal));
    printIds(pattern);
    printf("; ");
    for (size_t tId : transactionsList)
    {
        printf("%d ", uint32_t(tId));
    }
    printf("\n");
}

int main()
{
    printf("Generate dataset\n");
    Dataset dataset;
    //generateRandomDataSet(dataset, 500, 10, 40, 35);
    generateToyDataSet(dataset);

    // step1. Generate dataset mapping
    printf("Generate mapping\n");
    Mapping mapping = getDatasetMapping(dataset);
    size_t bitsetSize = mapping.indexToId.size();

    // step2. Convert all transactions to bitsets
    printf("Create bitsets\n");
    std::vector<Bitset> bitsetTransactions;
    for (const Dataset::Transaction& transaction : dataset.transactions)
    {
        Bitset bitset(bitsetSize);
        for (size_t bitIndex = 0; bitIndex < mapping.indexToId.size(); bitIndex++)
        {
            id_t bitId = mapping.indexToId[bitIndex];
            auto it = transaction.uniqueIds.find(bitId);
            if (it != transaction.uniqueIds.end())
            {
                bitset.set(bitIndex);
            }
            else
            {
                bitset.reset(bitIndex);
            }
        }
        bitsetTransactions.emplace_back(std::move(bitset));
    }

    // step3. Find bitsets intersections and accumulate.
    // Note: O(N^2) where N = number of transactions
    printf("Serch for freq. patterns\n");
    std::unordered_map<Bitset, size_t> patterns;
    for (size_t i = 0; i < bitsetTransactions.size(); i++)
    {
        printf("%3.2f %%                      \r", 100.0 * double(i+1) / double(bitsetTransactions.size()));

        const Bitset& a = bitsetTransactions[i];
        for (size_t j = i + 1; j < bitsetTransactions.size(); j++)
        {
            const Bitset& b = bitsetTransactions[j];
            Bitset matchingBits = Bitset::match(a, b);
            size_t numMatchedBits = matchingBits.count();
            // printf("Num matched bits %d (%d vs %d)\n", int(numMatchedBits), int(i), int(j));

            if (numMatchedBits < 3)
            {
                // the resulting pattern is too short - ignore
                continue;
            }

            //printf("Num matched bits %d (%d vs %d)\n", int(numMatchedBits), int(i), int(j));
            auto it = patterns.try_emplace(std::move(matchingBits), 1);
            if (!it.second)
            {
                // if exist increase "weight" value
                it.first->second++;
            }
        }
    }

    printf("\n");

    printf("Done\n");

    // step 4 (optional)
    // "Linearize" patterns and sort by popularity/length
    printf("Linearize/uncompress patters\n");
    struct Pattern
    {
        std::vector<id_t> data;
        std::vector<size_t> matches;
    };

    double kThreshold = 0.1f;
    size_t numSessionsThreshold = size_t(0.5 + double(dataset.transactions.size()) * double(kThreshold));
    numSessionsThreshold = std::max(numSessionsThreshold, size_t(1));
    numSessionsThreshold = std::min(numSessionsThreshold, dataset.transactions.size());

    std::vector<Pattern> linPatterns;
    for (auto it = patterns.begin(); it != patterns.end(); ++it)
    {
        size_t numMatches = _getNumTransactionsThatMatchPattern(bitsetTransactions, it->first);
        if (numMatches < numSessionsThreshold)
        {
            // this patter is too rare = skip
            continue;
        }

        Pattern& pattern = linPatterns.emplace_back();
        pattern.data = getPattern(it->first, mapping);
        pattern.matches = getTransactionsThatMatchPattern(dataset, pattern.data);
        assert(pattern.matches.size() == numMatches);
    }
    std::sort(linPatterns.begin(), linPatterns.end(), [](const Pattern& a, const Pattern& b) { return a.data.size() > b.data.size(); });

    // step5 (print results)
    printf("Print results\n");
    printDataset(dataset);

    printf("---------------\n");
    printf("%% matches; num matches ; pattern ; matched sessions\n");

    for (Pattern& pattern : linPatterns)
    {
        printPattern(pattern.data, pattern.matches, dataset.transactions.size());
    }

    return 0;
}
