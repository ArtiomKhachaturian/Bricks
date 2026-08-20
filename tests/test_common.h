#pragma once

#include <atomic>

namespace BricksTests {

class ITestListener {
public:
    virtual ~ITestListener() = default;
    virtual void onEvent(int value) = 0;
    virtual int lastValue() const = 0;
};

class TestListenerImpl final : public ITestListener {
public:
    void onEvent(int value) override {
        _callCount.fetch_add(1, std::memory_order_relaxed);
        _lastValue.store(value, std::memory_order_relaxed);
    }
    int lastValue() const override {
        return _lastValue.load(std::memory_order_relaxed);
    }
    int callCount() const {
        return _callCount.load(std::memory_order_relaxed);
    }
private:
    std::atomic<int> _callCount{0};
    std::atomic<int> _lastValue{0};
};

} // namespace BricksTests
