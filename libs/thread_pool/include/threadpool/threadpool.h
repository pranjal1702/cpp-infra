#include "mpmc/mpmc_mutex.h"
#include<cstdint>
#include <functional>
#include <future>
#include <utility>
#include <type_traits>
#include<thread>
#include<vector>
#include<mutex>
#include <condition_variable>
#include<atomic>
#include<exception>
#include<memory>

class queue_full_error : public std::runtime_error{
public:
    explicit queue_full_error(std::size_t capacity): std::runtime_error("Queue is full, task not submitted. Capacity = "+ std::to_string(capacity)){

    }
};

namespace threadpool{
    
template<std::size_t Workers_count,std::size_t Queue_Size=256>
class ThreadPool{
public:
    ThreadPool(){
        // create worker threads
        for(std::size_t i=0;i<Workers_count;i++){
            _workers.emplace_back([this]{
                _worker_loop();
            });
        }
    }
    template<typename F>
    std::future<std::invoke_result_t<std::decay_t<F>>> submit_task(F&& f){
        using ReturnType = std::invoke_result_t<std::decay_t<F>>;

        std::shared_ptr<std::packaged_task<ReturnType()>> packaged_task_shared_ptr = std::make_shared<std::packaged_task<ReturnType()>> (std::forward<F> (f));
        auto future = packaged_task_shared_ptr->get_future();
        bool res=false;
        {
            // this lock is needed so that when pool stops no job get submitted
            std::lock_guard<std::mutex> lock(_mtx);
            if (_stop)
                throw std::runtime_error("pool stopped");
        }
            res = _queue.push(
                [packaged_task_shared_ptr]() mutable {
                    (*packaged_task_shared_ptr)();
                }
            );
        // if queue is not empty then what to return, how to handle this fail push
        if(!res){
            throw queue_full_error(Queue_Size);
        }
        _cv.notify_one();
        return future;
    }

    ~ThreadPool(){
        {
            std::lock_guard<std::mutex> lock(_mtx);
            _stop = true;
        }

        _cv.notify_all();

        for(auto &worker: _workers){
            if(worker.joinable()){
                worker.join();
            }
        }
    }

private:
    mpmc::MPMC_Mutex<std::function<void()>,Queue_Size> _queue;
    std::vector<std::thread> _workers;
    std::condition_variable _cv;
    std::mutex _mtx;
    std::atomic<bool> _stop=false;
    void _worker_loop(){
        while(true){
            {
                std::function<void()> func;
                {
                    std::unique_lock<std::mutex> lock(_mtx);
                    _cv.wait(lock,[this]{
                        return !_queue.is_empty() || _stop.load();
                    });
                    if(_stop && _queue.is_empty()) break;
                }
                // no lock needed here as our queue is already thread safe
                auto opt_func=_queue.try_pop();
                if(opt_func.has_value()){
                    func=std::move(opt_func.value());
                }

                if(func)
                    func();

            }

        }
    }

};

} // threadpool namespace