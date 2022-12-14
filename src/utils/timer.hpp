
/*
 * Created on Sat Oct 29 2022
 *
 * Jose Melo - 2022
 */

#ifndef BDE52A8D_4879_4018_AB8C_85B3240CCE88
#define BDE52A8D_4879_4018_AB8C_85B3240CCE88

#include <chrono>
#include <iostream>

namespace utils
{
    class Timer {
       public:
        Timer() {
            startPoint = std::chrono::high_resolution_clock::now();
        };
        ~Timer() {
            auto endPoint = std::chrono::high_resolution_clock::now();
            auto start    = std::chrono::time_point_cast<std::chrono::microseconds>(startPoint).time_since_epoch().count();
            auto end      = std::chrono::time_point_cast<std::chrono::microseconds>(endPoint).time_since_epoch().count();
            std::cout << "Elapsed time: " << (end - start) << "us \n";
        };

       private:
        std::chrono::time_point<std::chrono::high_resolution_clock> startPoint;
    };
}  // namespace utils

#endif /* BDE52A8D_4879_4018_AB8C_85B3240CCE88 */
