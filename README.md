# FindFrequentPatterns

It is a draft idea for an algorithm for finding stable and very long patterns among many "transactions." (similar to the Apriori algorithm)
Unfortunately, the Apriori algorithm complexity is O(2^D), where D represents the horizontal width present in the database (and in case we are looking for long patterns, this number is extremely huge).

Another algorithm to solve this problem called MaxMiner
https://www2.cs.sfu.ca/CourseCentral/741/jpei/readings/baya98.pdf
and it was specifically designed to find long, stable patterns in the dataset.


The proposed approach is extremely simple. Represent every unique number in the data set as a single bit in a huge bitset and then use bitwise operations to find an intersection between different transactions. 

Naive implementation (this code) has O(N^2) complexity where N is the number of transactions to analyze, but I think we can improve it by slicing the input dataset into reasonably sized "mini-batches" (similar to what ML is doing) and to mine stable patterns from those mini-batches



