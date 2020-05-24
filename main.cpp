#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <future>
#include <chrono>
#include <atomic>

constexpr int NMAX = 10000000;

std::atomic<int> threads;

int partition(std::vector<int>& v, int l, int r)
{
    int pivot = v[(l+r)/2];
    int i = l;
    int j = r;
    while (i <= j)
    {
        while (v[i] < pivot)
            i++;
        while (v[j] > pivot)
            j--;
        if (i >= j)
            break;
        std::swap(v[i++], v[j--]);
    }
    return j;
}

void quicksortNoThread(std::vector<int>& v, int l, int r)
{
    if (l >= r)
        return;
    int q = partition(v, l, r);
   quicksortNoThread(v, l, q);
   quicksortNoThread(v, q + 1, r);

}

void quicksortAsync(std::vector<int>& v, int l, int r)
{
    if (l >= r)
        return;
    int q = partition(v, l, r);

    std::launch launch;
    if (threads > 0)
    {
        launch = std::launch::async;
        threads--;
    }
    else
    {
        launch = std::launch::deferred;
    }

    std::future<void> left = std::async(
        launch, quicksortAsync, std::ref(v), l, q);
    std::future<void> right = std::async(
        std::launch::deferred, quicksortAsync, std::ref(v), q + 1, r);

    left.get();
    if (launch == std::launch::async)
        threads++;

    right.get();
}

void quicksortPureThread(std::vector<int>& v, int l, int r)
{
    if (l >= r)
        return;
    int q = partition(v, l, r);
    if (threads.load() > 0)
    {
        std::thread left([&]() {quicksortPureThread(v, l, q); threads++; });
        threads--;
        quicksortPureThread(v, q + 1, r);
        left.join();
    } else
    {
        quicksortPureThread(v, l, q);
        quicksortPureThread(v, q + 1, r);
    }
    ;
}

void output(const std::vector<int>& v)
{
    for (auto& e : v)
        std::cout << e << " ";
    std::cout << "\n";
}

int main()
{
    std::vector<int> sample(NMAX);
    
    std::mt19937 rand(time(0));
    for (int i = 0; i < NMAX; i++)
        sample[i] = rand();


    {
        std::cout << "No multithreading: ";
        
        std::vector<int> testNoThread = sample;
        auto start = std::chrono::system_clock::now();
        quicksortNoThread(testNoThread, 0, testNoThread.size() - 1);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> timePassed = end - start;
        std::cout << "sorted " << NMAX << " elements in " << timePassed.count() << " seconds\n";
    }

    {
        std::cout << "Pure threads: ";

        threads = std::thread::hardware_concurrency();
        if (threads == 0) threads = 2;

        std::vector<int> test = sample;
        auto start = std::chrono::system_clock::now();
        quicksortPureThread(test, 0, test.size() - 1);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> timePassed = end - start;
        std::cout << "sorted " << NMAX << " elements in " << timePassed.count() << " seconds\n";
    }

    {
        std::cout << "Async: ";

        threads = std::thread::hardware_concurrency();
        if (threads == 0) threads = 2;

        std::vector<int> testThread = sample;
        auto start = std::chrono::system_clock::now();
        quicksortAsync(testThread, 0, testThread.size() - 1);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> timePassed = end - start;
        std::cout << "sorted " << NMAX << " elements in " << timePassed.count() << " seconds\n";
    }

    system("pause");
}