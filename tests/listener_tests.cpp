#include <gtest/gtest.h>

#include "listeners/Listener.h"
#include "test_common.h"

#include <atomic>
#include <memory>
#include <thread>

using BricksTests::ITestListener;
using BricksTests::TestListenerImpl;

// =============================================================================
// Bricks::Listener<ITestListener*>  (ThreadSafe = true, the default)
// =============================================================================
using RawListener = Bricks::Listener<ITestListener*>;

TEST(ListenerRawPtrTest, DefaultConstructedIsEmpty) {
    RawListener listener;
    EXPECT_TRUE(listener.empty());
    EXPECT_FALSE(static_cast<bool>(listener));
}

TEST(ListenerRawPtrTest, NullptrConstructedIsEmpty) {
    RawListener listener(nullptr);
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerRawPtrTest, ConstructWithValueIsNotEmpty) {
    TestListenerImpl impl;
    RawListener listener(&impl);
    EXPECT_FALSE(listener.empty());
    EXPECT_TRUE(static_cast<bool>(listener));
}

TEST(ListenerRawPtrTest, SetAssignsListener) {
    TestListenerImpl impl;
    RawListener listener;
    listener.set(&impl);
    EXPECT_FALSE(listener.empty());
}

TEST(ListenerRawPtrTest, ResetClearsListener) {
    TestListenerImpl impl;
    RawListener listener(&impl);
    listener.reset();
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerRawPtrTest, OperatorBoolReflectsEmptiness) {
    TestListenerImpl impl;
    RawListener listener;
    EXPECT_FALSE(static_cast<bool>(listener));
    listener.set(&impl);
    EXPECT_TRUE(static_cast<bool>(listener));
}

TEST(ListenerRawPtrTest, InvokeCallsMethodWhenSet) {
    TestListenerImpl impl;
    RawListener listener(&impl);
    listener.invoke(&ITestListener::onEvent, 42);
    EXPECT_EQ(1, impl.callCount());
    EXPECT_EQ(42, impl.lastValue());
}

TEST(ListenerRawPtrTest, InvokeIsNoopWhenEmpty) {
    RawListener listener;
    EXPECT_NO_THROW(listener.invoke(&ITestListener::onEvent, 7));
}

TEST(ListenerRawPtrTest, InvokeRReturnsValueWhenSet) {
    TestListenerImpl impl;
    RawListener listener(&impl);
    listener.invoke(&ITestListener::onEvent, 99);
    EXPECT_EQ(99, listener.invokeR<int>(&ITestListener::lastValue));
}

TEST(ListenerRawPtrTest, InvokeRReturnsDefaultWhenEmpty) {
    RawListener listener;
    EXPECT_EQ(0, listener.invokeR<int>(&ITestListener::lastValue));
}

TEST(ListenerRawPtrTest, AssignmentOperatorSetsListener) {
    TestListenerImpl impl;
    RawListener listener;
    listener = &impl;
    EXPECT_FALSE(listener.empty());
}

TEST(ListenerRawPtrTest, CopyConstructorSharesListener) {
    TestListenerImpl impl;
    RawListener original(&impl);
    RawListener copy(original);
    copy.invoke(&ITestListener::onEvent, 5);
    EXPECT_EQ(1, impl.callCount());
    EXPECT_FALSE(original.empty());
}

TEST(ListenerRawPtrTest, CopyAssignmentSharesListener) {
    TestListenerImpl impl;
    RawListener original(&impl);
    RawListener copy;
    copy = original;
    EXPECT_FALSE(copy.empty());
}

TEST(ListenerRawPtrTest, MoveConstructorTransfersListenerToDestination) {
    TestListenerImpl impl;
    RawListener original(&impl);
    RawListener moved(std::move(original));
    EXPECT_FALSE(moved.empty());
    moved.invoke(&ITestListener::onEvent, 1);
    EXPECT_EQ(1, impl.callCount());
    // Deliberately not asserting `original`'s state here -- see
    // KnownIssue_MoveDoesNotResetSourceForThreadSafeRawPointer below.
}

TEST(ListenerRawPtrTest, MoveAssignmentTransfersListenerToDestination) {
    TestListenerImpl impl;
    RawListener original(&impl);
    RawListener moved;
    moved = std::move(original);
    EXPECT_FALSE(moved.empty());
}

// KNOWN ISSUE: ListenerStorage<T*, true>::assign(&&) takes the source's value
// via SafeObj<T,...>::take(), which is `return std::move(_obj);` -- a no-op
// for a scalar T* (no pointer "move" nulls the source). Every other
// Listener<T,ThreadSafe> combination correctly empties the moved-from
// instance; this is the sole exception. This test documents *current*
// behavior as a regression guard, not the desired one -- if Bricks fixes
// ListenerStorage<T*,true>'s move, this assertion should be flipped to
// EXPECT_TRUE(original.empty()).
TEST(ListenerRawPtrTest, KnownIssue_MoveDoesNotResetSourceForThreadSafeRawPointer) {
    TestListenerImpl impl;
    RawListener original(&impl);
    RawListener moved(std::move(original));
    EXPECT_FALSE(moved.empty());
    EXPECT_FALSE(original.empty()); // documents the bug: source is NOT reset
}

// =============================================================================
// Bricks::Listener<ITestListener*, false>  (ThreadSafe = false)
// =============================================================================
using RawListenerUnsafe = Bricks::Listener<ITestListener*, false>;

TEST(ListenerRawPtrUnsafeTest, DefaultConstructedIsEmpty) {
    RawListenerUnsafe listener;
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerRawPtrUnsafeTest, SetAndResetWork) {
    TestListenerImpl impl;
    RawListenerUnsafe listener;
    listener.set(&impl);
    EXPECT_FALSE(listener.empty());
    listener.reset();
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerRawPtrUnsafeTest, InvokeCallsMethodWhenSet) {
    TestListenerImpl impl;
    RawListenerUnsafe listener(&impl);
    listener.invoke(&ITestListener::onEvent, 11);
    EXPECT_EQ(11, impl.lastValue());
}

TEST(ListenerRawPtrUnsafeTest, InvokeRReturnsValueWhenSet) {
    TestListenerImpl impl;
    RawListenerUnsafe listener(&impl);
    listener.invoke(&ITestListener::onEvent, 13);
    EXPECT_EQ(13, listener.invokeR<int>(&ITestListener::lastValue));
}

// Unlike the ThreadSafe=true specialization, this one correctly empties the
// source (ListenerStorage<T*,false>'s move ctor does `tmp._u = nullptr`).
TEST(ListenerRawPtrUnsafeTest, MoveConstructorTransfersListenerAndEmptiesSource) {
    TestListenerImpl impl;
    RawListenerUnsafe original(&impl);
    RawListenerUnsafe moved(std::move(original));
    EXPECT_FALSE(moved.empty());
    EXPECT_TRUE(original.empty());
}

// =============================================================================
// Bricks::Listener<std::shared_ptr<ITestListener>>  (ThreadSafe = true)
// =============================================================================
using SharedListener = Bricks::Listener<std::shared_ptr<ITestListener>>;

TEST(ListenerSharedPtrTest, DefaultConstructedIsEmpty) {
    SharedListener listener;
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerSharedPtrTest, NullptrConstructedIsEmpty) {
    SharedListener listener(nullptr);
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerSharedPtrTest, ConstructWithValueIsNotEmpty) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListener listener(impl);
    EXPECT_FALSE(listener.empty());
}

TEST(ListenerSharedPtrTest, SetAssignsListener) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListener listener;
    listener.set(impl);
    EXPECT_FALSE(listener.empty());
}

TEST(ListenerSharedPtrTest, ResetClearsListener) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListener listener(impl);
    listener.reset();
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerSharedPtrTest, OperatorBoolReflectsEmptiness) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListener listener;
    EXPECT_FALSE(static_cast<bool>(listener));
    listener.set(impl);
    EXPECT_TRUE(static_cast<bool>(listener));
}

TEST(ListenerSharedPtrTest, InvokeCallsMethodWhenSet) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListener listener(impl);
    listener.invoke(&ITestListener::onEvent, 21);
    EXPECT_EQ(21, impl->lastValue());
}

TEST(ListenerSharedPtrTest, InvokeIsNoopWhenEmpty) {
    SharedListener listener;
    EXPECT_NO_THROW(listener.invoke(&ITestListener::onEvent, 1));
}

TEST(ListenerSharedPtrTest, InvokeRReturnsValueWhenSet) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListener listener(impl);
    listener.invoke(&ITestListener::onEvent, 55);
    EXPECT_EQ(55, listener.invokeR<int>(&ITestListener::lastValue));
}

TEST(ListenerSharedPtrTest, InvokeRReturnsDefaultWhenEmpty) {
    SharedListener listener;
    EXPECT_EQ(0, listener.invokeR<int>(&ITestListener::lastValue));
}

TEST(ListenerSharedPtrTest, AssignmentOperatorSetsListener) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListener listener;
    listener = impl;
    EXPECT_FALSE(listener.empty());
}

TEST(ListenerSharedPtrTest, CopyConstructorSharesListener) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListener original(impl);
    SharedListener copy(original);
    copy.invoke(&ITestListener::onEvent, 3);
    EXPECT_EQ(1, impl->callCount());
    EXPECT_FALSE(original.empty());
}

TEST(ListenerSharedPtrTest, CopyAssignmentSharesListener) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListener original(impl);
    SharedListener copy;
    copy = original;
    EXPECT_FALSE(copy.empty());
}

TEST(ListenerSharedPtrTest, MoveConstructorTransfersListenerAndEmptiesSource) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListener original(impl);
    SharedListener moved(std::move(original));
    EXPECT_FALSE(moved.empty());
    EXPECT_TRUE(original.empty());
}

TEST(ListenerSharedPtrTest, MoveAssignmentTransfersListenerAndEmptiesSource) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListener original(impl);
    SharedListener moved;
    moved = std::move(original);
    EXPECT_FALSE(moved.empty());
    EXPECT_TRUE(original.empty());
}

// =============================================================================
// Bricks::Listener<std::shared_ptr<ITestListener>, false>  (ThreadSafe = false)
// =============================================================================
using SharedListenerUnsafe = Bricks::Listener<std::shared_ptr<ITestListener>, false>;

TEST(ListenerSharedPtrUnsafeTest, DefaultConstructedIsEmpty) {
    SharedListenerUnsafe listener;
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerSharedPtrUnsafeTest, SetAndResetWork) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListenerUnsafe listener;
    listener.set(impl);
    EXPECT_FALSE(listener.empty());
    listener.reset();
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerSharedPtrUnsafeTest, InvokeCallsMethodWhenSet) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListenerUnsafe listener(impl);
    listener.invoke(&ITestListener::onEvent, 8);
    EXPECT_EQ(8, impl->lastValue());
}

TEST(ListenerSharedPtrUnsafeTest, InvokeRReturnsValueWhenSet) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListenerUnsafe listener(impl);
    listener.invoke(&ITestListener::onEvent, 9);
    EXPECT_EQ(9, listener.invokeR<int>(&ITestListener::lastValue));
}

TEST(ListenerSharedPtrUnsafeTest, MoveConstructorTransfersListenerAndEmptiesSource) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListenerUnsafe original(impl);
    SharedListenerUnsafe moved(std::move(original));
    EXPECT_FALSE(moved.empty());
    EXPECT_TRUE(original.empty());
}

// =============================================================================
// Bricks::Listener<std::weak_ptr<ITestListener>>  (ThreadSafe = true)
// =============================================================================
using WeakListener = Bricks::Listener<std::weak_ptr<ITestListener>>;

TEST(ListenerWeakPtrTest, DefaultConstructedIsEmpty) {
    WeakListener listener;
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerWeakPtrTest, NullptrConstructedIsEmpty) {
    WeakListener listener(std::weak_ptr<ITestListener>{});
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerWeakPtrTest, ConstructWithValueIsNotEmpty) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListener listener{std::weak_ptr<ITestListener>(impl)};
    EXPECT_FALSE(listener.empty());
}

TEST(ListenerWeakPtrTest, SetAssignsListener) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListener listener;
    listener.set(std::weak_ptr<ITestListener>(impl));
    EXPECT_FALSE(listener.empty());
}

TEST(ListenerWeakPtrTest, ResetClearsListener) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListener listener{std::weak_ptr<ITestListener>(impl)};
    listener.reset();
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerWeakPtrTest, OperatorBoolReflectsEmptiness) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListener listener;
    EXPECT_FALSE(static_cast<bool>(listener));
    listener.set(std::weak_ptr<ITestListener>(impl));
    EXPECT_TRUE(static_cast<bool>(listener));
}

TEST(ListenerWeakPtrTest, InvokeCallsMethodWhenSet) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListener listener{std::weak_ptr<ITestListener>(impl)};
    listener.invoke(&ITestListener::onEvent, 17);
    EXPECT_EQ(17, impl->lastValue());
}

TEST(ListenerWeakPtrTest, InvokeIsNoopWhenEmpty) {
    WeakListener listener;
    EXPECT_NO_THROW(listener.invoke(&ITestListener::onEvent, 1));
}

TEST(ListenerWeakPtrTest, InvokeRReturnsValueWhenSet) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListener listener{std::weak_ptr<ITestListener>(impl)};
    listener.invoke(&ITestListener::onEvent, 61);
    EXPECT_EQ(61, listener.invokeR<int>(&ITestListener::lastValue));
}

TEST(ListenerWeakPtrTest, InvokeRReturnsDefaultWhenEmpty) {
    WeakListener listener;
    EXPECT_EQ(0, listener.invokeR<int>(&ITestListener::lastValue));
}

TEST(ListenerWeakPtrTest, AssignmentOperatorSetsListener) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListener listener;
    listener = std::weak_ptr<ITestListener>(impl);
    EXPECT_FALSE(listener.empty());
}

TEST(ListenerWeakPtrTest, CopyConstructorSharesListener) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListener original{std::weak_ptr<ITestListener>(impl)};
    WeakListener copy(original);
    copy.invoke(&ITestListener::onEvent, 6);
    EXPECT_EQ(1, impl->callCount());
    EXPECT_FALSE(original.empty());
}

TEST(ListenerWeakPtrTest, CopyAssignmentSharesListener) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListener original{std::weak_ptr<ITestListener>(impl)};
    WeakListener copy;
    copy = original;
    EXPECT_FALSE(copy.empty());
}

TEST(ListenerWeakPtrTest, MoveConstructorTransfersListenerAndEmptiesSource) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListener original{std::weak_ptr<ITestListener>(impl)};
    WeakListener moved(std::move(original));
    EXPECT_FALSE(moved.empty());
    EXPECT_TRUE(original.empty());
}

TEST(ListenerWeakPtrTest, MoveAssignmentTransfersListenerAndEmptiesSource) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListener original{std::weak_ptr<ITestListener>(impl)};
    WeakListener moved;
    moved = std::move(original);
    EXPECT_FALSE(moved.empty());
    EXPECT_TRUE(original.empty());
}

TEST(ListenerWeakPtrTest, EmptyAfterReferentDestroyed) {
    WeakListener listener;
    {
        auto impl = std::make_shared<TestListenerImpl>();
        listener.set(std::weak_ptr<ITestListener>(impl));
        EXPECT_FALSE(listener.empty());
    } // impl destroyed here
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerWeakPtrTest, InvokeIsNoopAfterReferentDestroyed) {
    WeakListener listener;
    {
        auto impl = std::make_shared<TestListenerImpl>();
        listener.set(std::weak_ptr<ITestListener>(impl));
    }
    EXPECT_NO_THROW(listener.invoke(&ITestListener::onEvent, 1));
}

// =============================================================================
// Bricks::Listener<std::weak_ptr<ITestListener>, false>  (ThreadSafe = false)
// =============================================================================
using WeakListenerUnsafe = Bricks::Listener<std::weak_ptr<ITestListener>, false>;

TEST(ListenerWeakPtrUnsafeTest, DefaultConstructedIsEmpty) {
    WeakListenerUnsafe listener;
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerWeakPtrUnsafeTest, SetAndResetWork) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListenerUnsafe listener;
    listener.set(std::weak_ptr<ITestListener>(impl));
    EXPECT_FALSE(listener.empty());
    listener.reset();
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerWeakPtrUnsafeTest, InvokeCallsMethodWhenSet) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListenerUnsafe listener{std::weak_ptr<ITestListener>(impl)};
    listener.invoke(&ITestListener::onEvent, 4);
    EXPECT_EQ(4, impl->lastValue());
}

TEST(ListenerWeakPtrUnsafeTest, InvokeRReturnsValueWhenSet) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListenerUnsafe listener{std::weak_ptr<ITestListener>(impl)};
    listener.invoke(&ITestListener::onEvent, 14);
    EXPECT_EQ(14, listener.invokeR<int>(&ITestListener::lastValue));
}

TEST(ListenerWeakPtrUnsafeTest, MoveConstructorTransfersListenerAndEmptiesSource) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListenerUnsafe original{std::weak_ptr<ITestListener>(impl)};
    WeakListenerUnsafe moved(std::move(original));
    EXPECT_FALSE(moved.empty());
    EXPECT_TRUE(original.empty());
}

TEST(ListenerWeakPtrUnsafeTest, EmptyAfterReferentDestroyed) {
    WeakListenerUnsafe listener;
    {
        auto impl = std::make_shared<TestListenerImpl>();
        listener.set(std::weak_ptr<ITestListener>(impl));
    }
    EXPECT_TRUE(listener.empty());
}

// =============================================================================
// Bricks::Listener<std::unique_ptr<ITestListener>>  (ThreadSafe = true)
// NOTE: unique_ptr is not copyable, so no copy-ctor/copy-assign cases here.
// =============================================================================
using UniqueListener = Bricks::Listener<std::unique_ptr<ITestListener>>;

TEST(ListenerUniquePtrTest, DefaultConstructedIsEmpty) {
    UniqueListener listener;
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerUniquePtrTest, NullptrConstructedIsEmpty) {
    UniqueListener listener(nullptr);
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerUniquePtrTest, ConstructWithValueIsNotEmpty) {
    UniqueListener listener(std::make_unique<TestListenerImpl>());
    EXPECT_FALSE(listener.empty());
}

TEST(ListenerUniquePtrTest, SetAssignsListener) {
    UniqueListener listener;
    listener.set(std::make_unique<TestListenerImpl>());
    EXPECT_FALSE(listener.empty());
}

TEST(ListenerUniquePtrTest, ResetClearsListener) {
    UniqueListener listener(std::make_unique<TestListenerImpl>());
    listener.reset();
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerUniquePtrTest, OperatorBoolReflectsEmptiness) {
    UniqueListener listener;
    EXPECT_FALSE(static_cast<bool>(listener));
    listener.set(std::make_unique<TestListenerImpl>());
    EXPECT_TRUE(static_cast<bool>(listener));
}

TEST(ListenerUniquePtrTest, InvokeCallsMethodWhenSet) {
    UniqueListener listener(std::make_unique<TestListenerImpl>());
    listener.invoke(&ITestListener::onEvent, 31);
    EXPECT_EQ(31, listener.invokeR<int>(&ITestListener::lastValue));
}

TEST(ListenerUniquePtrTest, InvokeIsNoopWhenEmpty) {
    UniqueListener listener;
    EXPECT_NO_THROW(listener.invoke(&ITestListener::onEvent, 1));
}

TEST(ListenerUniquePtrTest, InvokeRReturnsValueWhenSet) {
    UniqueListener listener(std::make_unique<TestListenerImpl>());
    listener.invoke(&ITestListener::onEvent, 44);
    EXPECT_EQ(44, listener.invokeR<int>(&ITestListener::lastValue));
}

TEST(ListenerUniquePtrTest, InvokeRReturnsDefaultWhenEmpty) {
    UniqueListener listener;
    EXPECT_EQ(0, listener.invokeR<int>(&ITestListener::lastValue));
}

TEST(ListenerUniquePtrTest, AssignmentOperatorSetsListener) {
    UniqueListener listener;
    listener = std::make_unique<TestListenerImpl>();
    EXPECT_FALSE(listener.empty());
}

TEST(ListenerUniquePtrTest, MoveConstructorTransfersListenerAndEmptiesSource) {
    UniqueListener original(std::make_unique<TestListenerImpl>());
    UniqueListener moved(std::move(original));
    EXPECT_FALSE(moved.empty());
    EXPECT_TRUE(original.empty());
}

TEST(ListenerUniquePtrTest, MoveAssignmentTransfersListenerAndEmptiesSource) {
    UniqueListener original(std::make_unique<TestListenerImpl>());
    UniqueListener moved;
    moved = std::move(original);
    EXPECT_FALSE(moved.empty());
    EXPECT_TRUE(original.empty());
}

// =============================================================================
// Bricks::Listener<std::unique_ptr<ITestListener>, false>  (ThreadSafe = false)
// =============================================================================
using UniqueListenerUnsafe = Bricks::Listener<std::unique_ptr<ITestListener>, false>;

TEST(ListenerUniquePtrUnsafeTest, DefaultConstructedIsEmpty) {
    UniqueListenerUnsafe listener;
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerUniquePtrUnsafeTest, SetAndResetWork) {
    UniqueListenerUnsafe listener;
    listener.set(std::make_unique<TestListenerImpl>());
    EXPECT_FALSE(listener.empty());
    listener.reset();
    EXPECT_TRUE(listener.empty());
}

TEST(ListenerUniquePtrUnsafeTest, InvokeCallsMethodWhenSet) {
    UniqueListenerUnsafe listener(std::make_unique<TestListenerImpl>());
    listener.invoke(&ITestListener::onEvent, 2);
    EXPECT_EQ(2, listener.invokeR<int>(&ITestListener::lastValue));
}

TEST(ListenerUniquePtrUnsafeTest, InvokeRReturnsValueWhenSet) {
    UniqueListenerUnsafe listener(std::make_unique<TestListenerImpl>());
    listener.invoke(&ITestListener::onEvent, 12);
    EXPECT_EQ(12, listener.invokeR<int>(&ITestListener::lastValue));
}

TEST(ListenerUniquePtrUnsafeTest, MoveConstructorTransfersListenerAndEmptiesSource) {
    UniqueListenerUnsafe original(std::make_unique<TestListenerImpl>());
    UniqueListenerUnsafe moved(std::move(original));
    EXPECT_FALSE(moved.empty());
    EXPECT_TRUE(original.empty());
}

// =============================================================================
// Concurrency smoke test (ThreadSafe = true, raw pointer)
// =============================================================================
TEST(ListenerConcurrencyTest, ConcurrentSetAndInvokeNoCrash) {
    TestListenerImpl implA;
    TestListenerImpl implB; // both kept alive for the whole test -- only the
                             // Listener's internal synchronization is racing,
                             // never a dangling pointee.
    RawListener listener(&implA);
    std::atomic<bool> stop{false};

    std::thread setter([&]() {
        int i = 0;
        while (!stop.load(std::memory_order_relaxed)) {
            listener.set(0 == (i++ % 2) ? &implA : &implB);
        }
    });
    std::thread invoker([&]() {
        for (int i = 0; i < 20000; ++i) {
            listener.invoke(&ITestListener::onEvent, i);
            (void)listener.invokeR<int>(&ITestListener::lastValue);
        }
    });

    invoker.join();
    stop.store(true, std::memory_order_relaxed);
    setter.join();

    // Pass criterion is "no crash/UB"; exact call counts are racy by design
    // and intentionally not asserted.
    SUCCEED();
}
