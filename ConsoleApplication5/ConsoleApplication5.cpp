// ConsoleApplication5.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <string>
#include <thread>

std::condition_variable g_cond; // 用來做通知
std::mutex g_mutex;             // 用來保護資料

// 產生隨機字串，只是個 Helper 函數，不是本範例重點
std::string generateRandomString() {
	std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	std::random_device rd;
	std::mt19937 generator(rd());
	std::uniform_int_distribution<int> distribution(0, characters.length() - 1);

	std::string result;
	int randomLength = distribution(generator);
	for (int i = 0; i < randomLength; ++i) {
		int randomIndex = distribution(generator);
		result += characters[randomIndex];
	}

	return result;
}

void run_producer_thread(std::queue<std::string> &queue) {
	for (;;) {
		std::cout << "P: === This is producer thread ===" << std::endl << std::flush;

		auto word = generateRandomString();

		std::lock_guard<std::mutex> lock{ g_mutex }; // 鎖住 queue 以免同時被 consumer 讀取
		queue.push(word);
		g_cond.notify_one(); // 通知 consumer
		std::cout << "P: producer just push a word: `" << word << "`" << std::endl << std::flush;

		std::cout << "P: producer sleep for 1500 ms" << std::endl << std::flush;
		std::this_thread::sleep_for(std::chrono::milliseconds(1500)); // 睡 1500 ms 方便觀察
	}
}

void run_consumer_thread(std::queue<std::string> &queue) {
	for (;;) {
		std::cout << "C: == This is consumer thread ==" << std::endl << std::flush;
		std::unique_lock<std::mutex> lock(g_mutex);        // 這邊的 lock 用來下一行檢查 queue 狀態
		g_cond.wait(lock, [&] { return !queue.empty(); }); // 當 queue 不為空時

		auto word = queue.front(); // 把資料拷貝出來，才能盡快解鎖
		queue.pop();

		lock.unlock(); // 解除 g_mutex 的鎖

		std::cout << "C: Consumer get a word `" << word << "`" << std::endl << std::flush;

		// 可以試試看取消註解來看影響，有什麼特別的情況，為什麼會這樣？
		std::cout << "C: consumer sleep for 1500 ms" << std::endl << std::flush;
		std::this_thread::sleep_for(std::chrono::milliseconds(1500)); // 睡 1500 ms 方便觀察
	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	std::queue<std::string> queue;

	auto producer_thread = std::thread([&queue]() { run_producer_thread(queue); });
	auto consumer_thread = std::thread([&queue]() { run_consumer_thread(queue); });

	producer_thread.join();
	consumer_thread.join();

	return 0;
}

