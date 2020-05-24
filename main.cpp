#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <future>
#include <chrono>
#include <atomic>

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
        std::thread left([&]() { threads--;  quicksortPureThread(v, l, q); threads++; });
        quicksortPureThread(v, q + 1, r);
        left.join();
    } else
    {
        quicksortPureThread(v, l, q);
        quicksortPureThread(v, q + 1, r);
    }
}

void output(const std::vector<int>& v)
{
    for (auto& e : v)
        std::cout << e << " ";
    std::cout << "\n";
}

int main()
{
    uint64_t NMAX{};
    std::cout << "Enter elements to sort (preferably between 1m and 100m): ";
    std::cin >> NMAX;

    std::vector<int> sample(NMAX);
    
    std::mt19937 rand(time(0));
    for (uint64_t i = 0; i < NMAX; i++)
        sample[i] = rand();

    {
        std::cout << "No multithreading: ";
        
        std::vector<int> test = sample;
        auto start = std::chrono::system_clock::now();
        quicksortNoThread(test, 0, test.size() - 1);
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

        std::vector<int> test = sample;
        auto start = std::chrono::system_clock::now();
        quicksortAsync(test, 0, test.size() - 1);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> timePassed = end - start;
        std::cout << "sorted " << NMAX << " elements in " << timePassed.count() << " seconds\n";
    }

    system("pause");
}