#pragma once
#include<cstdint>
#include<vector>
#include<mutex>
#include <condition_variable>
#include<optional>
namespace mpmc{

    // non-blocking queue
template<typename T,std::size_t Capacity>
class MPMC_Mutex{
    static_assert(
        (Capacity & (Capacity - 1)) == 0,
        "Capacity must be power of 2"
    );
    _buffer.resize(Capacity);
public:
    bool push(T val){
        {
            std::lock_guard<std::mutex> lock(_mtx);
            if(_count==Capacity){
                return false;
            }
            _buffer[_head] = std::move(val);
            _head=(_head+1)&(Capacity-1);
            _count++;
            return true;
        }
    }

    bool is_empty() const{
        {
            std::lock_guard<std::mutex> lock(_mtx);
            return _count==0;
        }
    }

    std::optional<T> try_pop(){
        {
            std::lock_guard<std::mutex> lock(_mtx);
            if(_count==0) return std::nullopt;
            T val(std::move(_buffer[_tail]));
            _tail=(_tail+1)&(Capacity-1);
            _count--;
            return val;
        }
    }
private:
    std::array<T, Capacity> _buffer;
    size_t _head=0;
    size_t _tail=0;
    size_t _count=0;
    std::mutex _mtx;
};


} // mpmc namespace ends