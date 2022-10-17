
#include <chrono>
#include <iostream>

namespace utils {
	class Timer
	{
	public:
		Timer() {
			startPoint = std::chrono::high_resolution_clock::now();
		};
		~Timer() {
			auto endPoint = std::chrono::high_resolution_clock::now();
			auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startPoint).time_since_epoch().count();
			auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endPoint).time_since_epoch().count();
			std::cout << "Elapsed time: " << (end - start) << "us \n";
		};

	private:
		std::chrono::time_point< std::chrono::high_resolution_clock> startPoint;
	};
}