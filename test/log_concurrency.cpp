#include "zeroerr/log.h"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

static zeroerr::String messageOnly(const zeroerr::LogMessage& message, bool) {
    return message.str() + "\n";
}

int main(int argc, char**) {
    zeroerr::setLogCustomCallback(messageOnly);
    const int count = argc > 1 ? 20000 : 1000;
    for (int threads : {1, 2, 4, 8}) {
        for (auto mode : {zeroerr::LogStream::FLUSH_AT_ONCE,
                          zeroerr::LogStream::FLUSH_WHEN_FULL,
                          zeroerr::LogStream::FLUSH_MANUALLY}) {
            const std::string path = "log-concurrency.txt";
            const auto start = std::chrono::steady_clock::now();
            {
                zeroerr::LogStream stream;
                stream.setFileLogger(path.c_str());
                stream.setFlushMode(mode);
                std::atomic<bool> done{false};
                std::thread flusher;
                if (argc == 1) flusher = std::thread([&] {
                    while (!done.load()) {
                        stream.flush();
                        std::this_thread::yield();
                    }
                });
                std::vector<std::thread> workers;
                for (int t = 0; t < threads; ++t) workers.emplace_back([&, t] {
                    for (int i = 0; i < count; ++i) {
                        ZEROERR_LOG("{id}", stream, t * count + i);
                    }
                });
                for (auto& worker : workers) worker.join();
                done = true;
                if (flusher.joinable()) flusher.join();
                stream.flush();
                // Force several block transitions without a concurrent flusher.
                if (argc == 1) {
                    for (int i = 0; i < count; ++i)
                        ZEROERR_LOG("{id}", stream, threads * count + i);
                }
            }
            auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
            std::ifstream input(path);
            std::vector<bool> seen((threads + (argc == 1 ? 1 : 0)) * count);
            int id, total = 0;
            while (input >> id) {
                if (id < 0 || id >= static_cast<int>(seen.size()) || seen[id]) return 1;
                seen[id] = true;
                ++total;
            }
            if (total != static_cast<int>(seen.size())) return 2;
            std::cout << threads << " threads, mode=" << mode << ": "
                      << total / elapsed << " records/s\n";
            std::remove(path.c_str());
        }
    }
    // Heap-owning arguments must be destroyed when blocks are recycled.
    {
        zeroerr::LogStream stream;
        stream.setFileLogger("log-owned.txt");
        stream.setFlushWhenFull();
        for (int i = 0; i < 3000; ++i) {
            ZEROERR_LOG("{text}", stream, zeroerr::String(256, 'x'));
        }
        stream.flush();
    }
    std::ifstream owned("log-owned.txt");
    std::string line;
    int lines = 0;
    while (std::getline(owned, line)) {
        if (line != std::string(256, 'x')) return 3;
        ++lines;
    }
    owned.close();
    std::remove("log-owned.txt");
    if (lines != 3000) return 4;

}
