#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <random>
#include <chrono>
#include <cmath>

using namespace std::chrono_literals;

int core_count = std::thread::hardware_concurrency();

void print_core_count()
{
    std::cout << "Количество аппаратных ядер - " << core_count << "\n\t\t1000\t\t10000\t\t100000\t\t1000000" << std::endl;
}

std::once_flag o_flag;

void vector_summ(const std::vector <int>& src1, const std::vector <int>& src2, std::vector<int>& out, const int start, const int stop)
{
    call_once(o_flag, print_core_count);
    for (int i = start; i < stop; ++i)
    {
        out.at(i) = src1.at(i) + src2.at(i);
    }
    //std::cout << "Поток завершен. Обрабатывали элементы с " << start << " по " << stop - 1  << std::endl;
}


int main()
{
    setlocale(LC_ALL, "Russian");
    std::mt19937 gen;
    std::uniform_int_distribution<int> dis(0, 32768);
    auto rand_num([=]() mutable {return dis(gen); });
    for (int threads_count = 1; threads_count <= 16; threads_count*=2)
    {
        std::vector<double> durations;
        for (int vector_size = 1000; vector_size <= 1000000; vector_size *= 10)
        {
            std::vector <int> src1(vector_size), src2(vector_size), out(vector_size);
            std::generate(src1.begin(), src1.end(), rand_num);
            std::generate(src2.begin(), src2.end(), rand_num);
            std::vector<std::thread> threads;

            auto start = std::chrono::high_resolution_clock::now();

            for (int start_index = 0; start_index < vector_size; start_index += ceil(static_cast<double>(vector_size) / threads_count))
            {
                int stop_index = start_index + ceil(static_cast<double>(vector_size) / threads_count); //Из-за того, что 1000 элементов разделить на 16 потоков поровну не получается
                if (stop_index > vector_size)
                {
                    stop_index = vector_size;
                }
                threads.push_back(std::thread(vector_summ, std::ref(src1), std::ref(src1), std::ref(out), start_index, stop_index));
            }
            for (auto& t : threads)
            {
                t.join();
            }
            auto stop = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> time = stop - start;
            durations.push_back(time.count());
        }
        std::cout << threads_count << " потоков\t";
        for (auto& d : durations)
            {
                std::cout << d << "\t\t";
            }
        std::cout << std::endl;
    }
}
