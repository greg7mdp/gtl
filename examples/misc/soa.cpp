/*
 *  Copyright (c) 2018 Mark Liu
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 */

/*
 *  Author: Mark Liu
 *
 *  Modified by: Gregory Popovitch
 *
 */

#include <cstring>
#include <gtl/soa.hpp>
#include <iostream>

int main()
{
    // presidents will be a soa representing  order, first name, last name

    constexpr int order      = 0;
    constexpr int first_name = 1;
    constexpr int last_name  = 2;

    gtl::soa<int, std::string, std::string> presidents;

    presidents.insert(0, "Abraham", "Lincoln");
    presidents.insert(3, "Barack", "Obama");
    presidents.insert(2, "George", "Bush");
    presidents.insert(1, "Bill", "Clinton");
    presidents.insert(4, "Donald", "Trump");
    presidents.insert(5, "Joe", "Biden");

    std::cout << "Presidents in order of insertion"
              << "\n";
    std::cout << presidents << "\n";

    // sort by time (first column)
    presidents.sort_by_field<order>();
    std::cout << "Presidents sorted by temporal order"
              << "\n";
    std::cout << presidents << "\n";

    // sort by first name (second column)
    presidents.sort_by_field<first_name>();
    std::cout << "Presidents sorted by first name"
              << "\n";
    std::cout << presidents << "\n";

    // sort by last name (third column)
    presidents.sort_by_field<last_name>();
    std::cout << "Presidents sorted by last name"
              << "\n";
    std::cout << presidents << "\n";

    // operator[] returns a tuple of references
    // Let's update Joe Biden to Joseph Biden
    {
        std::cout << "Editing the first row to update Joe => Joseph"
                  << "\n";
        auto [_, fname, lname] = presidents[0];
        fname                  = "Joseph"; // Joe => Joseph
        std::cout << presidents << "\n";
    }

    // view is templated to return a subset of fields
    // Let's update Abraham Lincoln to George Washington
    {
        std::cout << "Editing the third row to update Abraham Lincoln => George Washington"
                  << "\n";
        auto [fname, lname] = presidents.view<first_name, last_name>(3);
        fname               = "George";
        lname               = "Washington";
        std::cout << presidents << "\n";
    }

    // get_column<idx> returns direct access
    // to the underlying std::vector.
    // Let's sum the characters of the first names.
    std::cout << "Summing first name lengths\n";
    size_t length_sum = 0;
    for (const auto& fname : presidents.get_column<first_name>()) {
        length_sum += fname.length();
    }
    std::cout << "Total characters used in first names = " << length_sum << "\n\n";

    // We can pass a custom comparator when sorting
    // Let's sort based on length of last name
    std::cout << "Sorting by number of characters in the last name."
              << "\n";
    presidents.sort_by_field<last_name>(
        [](const auto& lname_a, const auto& lname_b) { return lname_a.size() < lname_b.size(); });
    std::cout << presidents << "\n";

    return 0;
}
