#ifndef TSDEQUE_H_
#define TSDEQUE_H_
#include <deque>
#include <mutex>

template <typename T>
class TsDeque{
    public:
        T pop_front(){
            const std::lock_guard<std::mutex> lock(mu);
            return q.pop_front();
        };
        T pop_back(){
            const std::lock_guard<std::mutex> lock(mu);
            return q.pop_back();
        };
        template <typename Type>
        T at(Type i){
            const std::lock_guard<std::mutex> lock(mu);
            return q.at(i);
        };
    protected:
        void testMeth();
        std::deque<T> q;
        std::mutex mu;

};

template <typename T>
void TsDeque<T>::testMeth(){
    q.pop_back();
    q.pop_front();
    q.at(1);
}

#endif // TSDEQUE_H_
