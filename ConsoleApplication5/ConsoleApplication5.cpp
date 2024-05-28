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

std::condition_variable g_cond; // �ΨӰ��q��
std::mutex g_mutex;             // �ΨӫO�@���

// �����H���r��A�u�O�� Helper ��ơA���O���d�ҭ��I
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

		std::lock_guard<std::mutex> lock{ g_mutex }; // ��� queue �H�K�P�ɳQ consumer Ū��
		queue.push(word);
		g_cond.notify_one(); // �q�� consumer
		std::cout << "P: producer just push a word: `" << word << "`" << std::endl << std::flush;

		std::cout << "P: producer sleep for 1500 ms" << std::endl << std::flush;
		std::this_thread::sleep_for(std::chrono::milliseconds(1500)); // �� 1500 ms ��K�[��
	}
}

void run_consumer_thread(std::queue<std::string> &queue) {
	for (;;) {
		std::cout << "C: == This is consumer thread ==" << std::endl << std::flush;
		std::unique_lock<std::mutex> lock(g_mutex);        // �o�䪺 lock �ΨӤU�@���ˬd queue ���A
		g_cond.wait(lock, [&] { return !queue.empty(); }); // �� queue �����Ů�

		auto word = queue.front(); // ���ƫ����X�ӡA�~��ɧָ���
		queue.pop();

		lock.unlock(); // �Ѱ� g_mutex ����

		std::cout << "C: Consumer get a word `" << word << "`" << std::endl << std::flush;

		// �i�H�ոլݨ������ѨӬݼv�T�A������S�O�����p�A������|�o�ˡH
		std::cout << "C: consumer sleep for 1500 ms" << std::endl << std::flush;
		std::this_thread::sleep_for(std::chrono::milliseconds(1500)); // �� 1500 ms ��K�[��
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

