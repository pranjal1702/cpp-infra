#include "mpmc-mutex-queue/mpmc-mutex.h"
#include<cstdint>
#include <functional>
#include <future>
#include <utility>
#include <type_traits>
#include<thread>
#include<vector>
#include<mutex>
#include<conditional_variable>
#include<atomic>
namespace threadpool{
    
template<std::size_t Workers_count>
class ThreadPool{
public:
    explicit ThreadPool(){
        // create worker threads
    }
    template<typename F>
    std::future<std::invoke_result_t<std::decay_t<F>>> submit_task(F&& f){
        using ReturnType = std::future<std::invoke_result_t<std::decay_t<F>>>

        std::packaged_task<ReturnType()> task(
            std::forward<F>(f)
        );

        auto future = task.get_future();

        bool res = _queue.push(
            [task = std::move(task)]() mutable {
                task();
            }
        );
        // if queue is not empty then what to return, how to handle this fail push
        return future;
    }
private:
    mpmc::MPMC_Mutex<std::function<void()>,256> _queue;
    std::vector<std::thread>> _workers;
    std::conditional_variable _cv;
    std::mutex _mtx;
    std::atomic<bool> _stop=false;
    void _worker_loop(){
        while(true){
            {
                std::lock_guard<std::mutex> lock(_mtx);
                _cv.wait(lock,[]{
                    return !_queue.is_empty() || _stop;
                });

                if(_stop && _queue.is_empty()) break;
                // no lock needed here as our queue is already thread safe
                auto opt_func=_queue.try_pop();
                if(opt_func.has_value()){
                    auto func=opt_func.value();
                    // execute this function
                    func();
                }

            }

        }
    }

};

} // threadpool namespace