#ifndef TSDEQUE_H_
#define TSDEQUE_H_
#include <deque>
#include <mutex>

template <typename T>
class TsDeque{
    public:
        const T& front(){
            const std::lock_guard<std::mutex> lock(mu);
            return q.front();
            }
        const T& back(){
            const std::lock_guard<std::mutex> lock(mu);
            return q.back();
        }
        T pop_front(){
            const std::lock_guard<std::mutex> lock(mu);
            auto t = std::move(q.front());
            q.pop_front();
            return t;
        };
        T pop_back(){
            const std::lock_guard<std::mutex> lock(mu);
            auto t = std::move(q.back());
            q.pop_back();
            return t;
        };
        void push_back(T val){
            const std::lock_guard<std::mutex> lock(mu);
            q.push_back(val);
        }
        void push_front(T val){
            const std::lock_guard<std::mutex> lock(mu);
            q.push_front(val);
        }
        template <typename Type>
        T at(Type i){
            const std::lock_guard<std::mutex> lock(mu);
            return q.at(i);
        };
        const bool& empty(){
            return q.empty();
        };
    protected:
        std::deque<T> q;
        std::mutex mu;

};


#endif // TSDEQUE_H_
