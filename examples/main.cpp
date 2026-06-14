#include <threadpool/threadpool.h>

#include <chrono>
#include <iostream>
#include <atomic>

int main()
{
    constexpr int N = 1'000'000;

    threadpool::ThreadPool<4,65536> pool;

    std::atomic<int> counter{0};

    auto start = std::chrono::steady_clock::now();

    std::vector<std::future<void>> futures;
    futures.reserve(N);

    std::size_t accepted = 0;
    std::size_t rejected = 0;

    for(int i=0;i<N;i++)
    {
        try{
            pool.submit_task([&]{
                counter.fetch_add(
                    1,
                    std::memory_order_relaxed
                );
            });
            accepted++;
        }
        catch (queue_full_error  &e)
        {
            ++rejected;
        }
    }

    std::cout<<"accepted "<<accepted<<'\n';
    std::cout<<"rejected "<<rejected<<'\n';

    while(counter.load() != accepted)
    {
        std::this_thread::yield();
    }

    // for (auto& f : futures)
    //     f.get();

    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Tasks: " << N << "\n";
    std::cout << "Time : " << elapsed.count() << " s\n";
    std::cout << "Throughput: "
              << N / elapsed.count()
              << " tasks/sec\n";
}