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

#include <iostream>
#include <cstdint>
#include <random>
#include <array>
#include <string>
#include <gtl/soa.hpp>
#include "tictoc.hpp"

using Id = unsigned short;

std::default_random_engine             gen;
std::uniform_int_distribution<Id>      sensor_id_gen(0, 100);
std::uniform_int_distribution<Id>      object_id_gen(0, 10);
std::uniform_int_distribution<int>     char_gen('a', 'z');
std::uniform_real_distribution<double> real_gen(-10.0, 10.0);

struct ArraySensorData
{
    std::array<double, 18> xyz;

    static ArraySensorData random()
    {
        ArraySensorData s;
        for (size_t i = 0; i < s.xyz.size(); ++i) {
            s.xyz[i] = real_gen(gen);
        }
        return s;
    };
};

struct StringSensorData
{
    std::string data;

    static StringSensorData random()
    {
        StringSensorData res;
        char             buff[31];
        for (size_t i = 0; i < 30; ++i) {
            buff[i] = char_gen(gen);
        }
        buff[30] = 0;
        res.data = buff;
        return res;
    };
};

template<typename... Ts>
std::ostream& operator<<(std::ostream& cout, const ArraySensorData& s)
{
    cout << "{" << s.xyz[0] << ", " << s.xyz[1] << ", " << s.xyz[2] << "}";
    return cout;
}

template<typename... Ts>
std::ostream& operator<<(std::ostream& cout, const StringSensorData& s)
{
    cout << "\"" << s.data << "\"";
    return cout;
}

using SensorData = StringSensorData;

struct Measurement
{
    Id         sensor_id;
    Id         object_id;
    double     timestamp;
    SensorData data;

    static Measurement random()
    {
        Measurement m;
        m.sensor_id = sensor_id_gen(gen);
        m.object_id = object_id_gen(gen);
        m.timestamp = real_gen(gen);
        m.data      = SensorData::random();
        return m;
    }
};

template<typename... Ts>
std::ostream& operator<<(std::ostream& cout, const Measurement& m)
{
    cout << "Measurement{" << m.sensor_id << ", " << m.object_id << ", " << m.timestamp << ", " << m.data << "}";
    return cout;
}

struct TestCase
{
    gtl::soa<Id, Id, double, SensorData> measurements_soa;
    std::vector<Measurement>             measurements_vec;

    static TestCase random()
    {
        TestCase t;
        for (int i = 0; i < 200000; ++i) {
            Measurement m = Measurement::random();
            t.measurements_vec.push_back(m);
            t.measurements_soa.insert(m.sensor_id, m.object_id, m.timestamp, m.data);
        }
        return t;
    }
};

int main(int argc, char* argv[])
{
    TicToc   tictoc;
    double   soa_sort_time   = 0;
    double   soa_ts_avg_time = 0;
    double   vec_sort_time   = 0;
    double   vec_ts_avg_time = 0;
    TestCase t0              = TestCase::random();
    t0.measurements_soa.prepare_tmp();

    int num_trials = 50;
    for (int trial = 0; trial <= num_trials; ++trial) {
        std::cout << "\rrunning trial " << trial << "/" << num_trials << std::flush;
        TestCase t = t0;

        t.measurements_soa.sort_by_field<1>();

        // soa sort
        tictoc.tic();
        t.measurements_soa.sort_by_field<0>();
        soa_sort_time += tictoc.toc();

        // soa sum
        tictoc.tic();
        double soa_ts_avg = 0;
        for (double d : t.measurements_soa.get_column<2>()) {
            soa_ts_avg += d;
        }

        soa_ts_avg /= t.measurements_soa.size();
        soa_ts_avg_time += tictoc.toc();

        // vec sort
        std::stable_sort(t.measurements_vec.begin(), t.measurements_vec.end(), [](auto& m1, auto& m2) {
            return m1.object_id < m2.object_id;
        });

        tictoc.tic();
        std::stable_sort(t.measurements_vec.begin(), t.measurements_vec.end(), [](auto& m1, auto& m2) {
            return m1.sensor_id < m2.sensor_id;
        });
        vec_sort_time += tictoc.toc();

        // vec sum
        tictoc.tic();
        double vec_ts_avg = 0;
        for (const auto& meas : t.measurements_vec) {
            vec_ts_avg += meas.timestamp;
        }
        vec_ts_avg /= t.measurements_vec.size();
        vec_ts_avg_time += tictoc.toc();

        if (argc == 1234) {
            // prevent optimization
            std::cout << t.measurements_soa.get_column<0>()[0] << "\n";
            std::cout << t.measurements_vec[0].timestamp << "\n";
            std::cout << vec_ts_avg << "\n";
            std::cout << soa_ts_avg << "\n";
        }
    }
    std::cout << "\rbenchmark results ==============" << std::endl;

    std::cout << "soa sort time " << soa_sort_time << std::endl;
    std::cout << "vec sort time " << vec_sort_time << std::endl;

    std::cout << "soa timestamp avg time " << soa_ts_avg_time << std::endl;
    std::cout << "vec timestamp avg time " << vec_ts_avg_time << std::endl;

    return 0;
}
