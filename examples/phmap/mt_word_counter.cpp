#include <iostream>
#include <fstream>
#include <sstream>
#include <gtl/phmap.hpp>
#include <gtl/btree.hpp>
#include <gtl/stopwatch.hpp>
#include <thread>
#include <array>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <filesystem>

/*
 * count the number of occurrences of each word in a large text file using multiple threads
 */

int main()
{
    const std::string filename = "1342-0.txt";

    if (!std::filesystem::exists(filename)) {
        // download Jane Austin "Pride and Prejudice"
        // ------------------------------------------
        if (system("curl https://www.gutenberg.org/files/1342/1342-0.txt -o 1342-0.txt") != 0) {
            std::cout << "Error: could not retrieve test file https://www.gutenberg.org/files/1342/1342-0.txt\n";
            return 1;
        }
    }

    constexpr size_t                                  num_threads = 16;
    std::array<std::vector<std::string>, num_threads> lines_array;

    {
        // populate num_threads vectors with lines from the book
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "Error: could not open file " << filename << '\n';
            return 1;
        }
        size_t      line_idx = 0;
        std::string line;
        while (std::getline(file, line)) {
            std::replace_if(
                line.begin(), line.end(), [](char c) -> bool { return !std::isalnum(c); }, ' ');
            lines_array[line_idx % num_threads].push_back(std::move(line));
            ++line_idx;
        }
    }

    using Map =
        gtl::parallel_flat_hash_map_m<std::string, size_t>; // parallel_flat_hash_map_m has default internal mutex
    Map word_counts;

    gtl::stopwatch sw(true);

    // run 16 threads, each thread processing lines from one of the vectors
    // -------------------------------------------------------------------
    constexpr size_t num_times = 256;
    for (size_t x = 0; x < num_times; ++x) {
        std::vector<std::thread> threads;
        threads.reserve(num_threads);
        for (size_t i = 0; i < num_threads; ++i) {
            threads.emplace_back(
                [&word_counts](const std::vector<std::string>& lines) {
                    for (const auto& line : lines) {
                        std::istringstream iss(line);
                        std::string        word;
                        while (iss >> word) {
                            // use lazy_emplace to modify the map while the mutex is locked
                            word_counts.lazy_emplace_l(
                                word,
                                [&](Map::value_type& p) { ++p.second; }, // called only when key was already present
                                [&](const Map::constructor& ctor) // construct value_type in place when key not present
                                { ctor(std::move(word), 1); });
                        }
                    }
                },
                lines_array[i]);
        }

        for (auto& thread : threads)
            thread.join();
    }

    sw.snap();

    // print one word used at each frequency
    // -------------------------------------
    gtl::btree_map<size_t, std::string> result;
    for (const auto& [word, freq] : word_counts)
        result[freq] = word;

    for (const auto& [freq, word] : result)
        std::cout << (freq / num_times) << ": " << word << '\n';

    printf("\n\nphmap time: %10.2fs\n", sw.start_to_snap() / 1000);
    return 0;
}
