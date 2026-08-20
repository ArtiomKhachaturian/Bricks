#include <gtest/gtest.h>

#include "listeners/Listeners.h"
#include "test_common.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

using BricksTests::ITestListener;
using BricksTests::TestListenerImpl;

// =============================================================================
// IMPORTANT: Listeners<T,ThreadSafe>::invoke() and ::apply() are NOT tested
// here for ANY T/ThreadSafe combination. Listeners' backing Container is
// std::vector<std::pair<uint64_t,T>>, but invoke()/apply() pass that
// container directly to Invoke<T>::make / Invoke<T>::apply, which only
// accept `const T&` / `const std::vector<T>&`. This does not compile for any
// T (confirmed with a minimal repro), and on Apple clang 21 attempting to
// compile the real call even crashes the compiler (SIGILL) while formatting
// the overload-resolution diagnostic. Do not add calls to .invoke()/.apply()
// here until this is fixed upstream in include/listeners/Listeners.h.
// =============================================================================

// =============================================================================
// Bricks::Listeners<ITestListener*>  (ThreadSafe = true, the default)
// =============================================================================
using RawListeners = Bricks::Listeners<ITestListener*>;

TEST(ListenersRawPtrTest, AddReturnsNonZeroIdAndWasAdded1stOnFirstInsertion) {
    TestListenerImpl impl;
    RawListeners listeners;
    bool wasAdded1st = false;
    const auto id = listeners.add(&impl, &wasAdded1st);
    EXPECT_NE(0ULL, id);
    EXPECT_TRUE(wasAdded1st);
    EXPECT_EQ(1u, listeners.size());
}

TEST(ListenersRawPtrTest, AddSecondListenerReturnsDistinctIdAndWasAdded1stFalse) {
    TestListenerImpl implA;
    TestListenerImpl implB;
    RawListeners listeners;
    const auto id1 = listeners.add(&implA);
    bool wasAdded1st = false; // caller must pre-init false; add() only ever sets true
    const auto id2 = listeners.add(&implB, &wasAdded1st);
    EXPECT_NE(0ULL, id2);
    EXPECT_NE(id1, id2);
    EXPECT_FALSE(wasAdded1st);
    EXPECT_EQ(2u, listeners.size());
}

TEST(ListenersRawPtrTest, AddReturnsZeroForNullListenerAndSizeUnchanged) {
    RawListeners listeners;
    const auto id = listeners.add(nullptr);
    EXPECT_EQ(0ULL, id);
    EXPECT_TRUE(listeners.empty());
}

TEST(ListenersRawPtrTest, ContainsReturnsTrueForAddedIdAndFalseForUnknownId) {
    TestListenerImpl impl;
    RawListeners listeners;
    const auto id = listeners.add(&impl);
    EXPECT_TRUE(listeners.contains(id));
    EXPECT_FALSE(listeners.contains(id + 1));
}

TEST(ListenersRawPtrTest, RemoveReturnsTrueAndErasesListener) {
    TestListenerImpl impl;
    RawListeners listeners;
    const auto id = listeners.add(&impl);
    EXPECT_TRUE(listeners.remove(id));
    EXPECT_FALSE(listeners.contains(id));
    EXPECT_TRUE(listeners.empty());
}

TEST(ListenersRawPtrTest, RemoveUnknownIdReturnsFalse) {
    RawListeners listeners;
    EXPECT_FALSE(listeners.remove(123456ULL));
}

TEST(ListenersRawPtrTest, WasRemovedLastTrueOnlyWhenContainerBecomesEmpty) {
    TestListenerImpl implA;
    TestListenerImpl implB;
    RawListeners listeners;
    const auto id1 = listeners.add(&implA);
    const auto id2 = listeners.add(&implB);

    bool wasRemovedLast = true; // sentinel; expect false first
    listeners.remove(id1, &wasRemovedLast);
    EXPECT_FALSE(wasRemovedLast);

    wasRemovedLast = false;
    listeners.remove(id2, &wasRemovedLast);
    EXPECT_TRUE(wasRemovedLast);
}

TEST(ListenersRawPtrTest, ClearReturnsTrueWhenNonEmptyThenFalseWhenAlreadyEmpty) {
    TestListenerImpl impl;
    RawListeners listeners;
    listeners.add(&impl);
    EXPECT_TRUE(listeners.clear());
    EXPECT_FALSE(listeners.clear());
}

TEST(ListenersRawPtrTest, EmptySizeAndOperatorBoolReflectContents) {
    TestListenerImpl impl;
    RawListeners listeners;
    EXPECT_TRUE(listeners.empty());
    EXPECT_EQ(0u, listeners.size());
    EXPECT_FALSE(static_cast<bool>(listeners));
    listeners.add(&impl);
    EXPECT_FALSE(listeners.empty());
    EXPECT_EQ(1u, listeners.size());
    EXPECT_TRUE(static_cast<bool>(listeners));
}

TEST(ListenersRawPtrTest, WasAdded1stTrueAgainAfterContainerEmptiedAndRefilled) {
    TestListenerImpl impl;
    RawListeners listeners;
    bool wasAdded1st = false;
    const auto id = listeners.add(&impl, &wasAdded1st);
    EXPECT_TRUE(wasAdded1st);
    listeners.remove(id);
    ASSERT_TRUE(listeners.empty());
    wasAdded1st = false;
    listeners.add(&impl, &wasAdded1st);
    EXPECT_TRUE(wasAdded1st);
}

// =============================================================================
// Bricks::Listeners<ITestListener*, false>  (ThreadSafe = false)
// =============================================================================
using RawListenersUnsafe = Bricks::Listeners<ITestListener*, false>;

TEST(ListenersRawPtrUnsafeTest, AddReturnsNonZeroIdAndWasAdded1stOnFirstInsertion) {
    TestListenerImpl impl;
    RawListenersUnsafe listeners;
    bool wasAdded1st = false;
    const auto id = listeners.add(&impl, &wasAdded1st);
    EXPECT_NE(0ULL, id);
    EXPECT_TRUE(wasAdded1st);
}

TEST(ListenersRawPtrUnsafeTest, RemoveReturnsTrueAndErasesListener) {
    TestListenerImpl impl;
    RawListenersUnsafe listeners;
    const auto id = listeners.add(&impl);
    EXPECT_TRUE(listeners.remove(id));
    EXPECT_TRUE(listeners.empty());
}

TEST(ListenersRawPtrUnsafeTest, RemoveUnknownIdReturnsFalse) {
    RawListenersUnsafe listeners;
    EXPECT_FALSE(listeners.remove(42ULL));
}

TEST(ListenersRawPtrUnsafeTest, ClearReturnsTrueWhenNonEmptyThenFalseWhenAlreadyEmpty) {
    TestListenerImpl impl;
    RawListenersUnsafe listeners;
    listeners.add(&impl);
    EXPECT_TRUE(listeners.clear());
    EXPECT_FALSE(listeners.clear());
}

TEST(ListenersRawPtrUnsafeTest, MoveConstructorTransfersListenersAndEmptiesSource) {
    TestListenerImpl impl;
    RawListenersUnsafe original;
    original.add(&impl);
    RawListenersUnsafe moved(std::move(original));
    EXPECT_EQ(1u, moved.size());
    EXPECT_TRUE(original.empty());
}

TEST(ListenersRawPtrUnsafeTest, MoveAssignmentTransfersListenersAndEmptiesSource) {
    TestListenerImpl impl;
    RawListenersUnsafe original;
    original.add(&impl);
    RawListenersUnsafe moved;
    moved = std::move(original);
    EXPECT_EQ(1u, moved.size());
    EXPECT_TRUE(original.empty());
}

// =============================================================================
// Bricks::Listeners<std::shared_ptr<ITestListener>>  (ThreadSafe = true)
// =============================================================================
using SharedListeners = Bricks::Listeners<std::shared_ptr<ITestListener>>;

TEST(ListenersSharedPtrTest, AddReturnsNonZeroIdAndWasAdded1stOnFirstInsertion) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListeners listeners;
    bool wasAdded1st = false;
    const auto id = listeners.add(impl, &wasAdded1st);
    EXPECT_NE(0ULL, id);
    EXPECT_TRUE(wasAdded1st);
    EXPECT_EQ(1u, listeners.size());
}

TEST(ListenersSharedPtrTest, AddSecondListenerReturnsDistinctIdAndWasAdded1stFalse) {
    auto implA = std::make_shared<TestListenerImpl>();
    auto implB = std::make_shared<TestListenerImpl>();
    SharedListeners listeners;
    const auto id1 = listeners.add(implA);
    bool wasAdded1st = false; // caller must pre-init false; add() only ever sets true
    const auto id2 = listeners.add(implB, &wasAdded1st);
    EXPECT_NE(id1, id2);
    EXPECT_FALSE(wasAdded1st);
    EXPECT_EQ(2u, listeners.size());
}

TEST(ListenersSharedPtrTest, AddReturnsZeroForNullListenerAndSizeUnchanged) {
    SharedListeners listeners;
    const auto id = listeners.add(nullptr);
    EXPECT_EQ(0ULL, id);
    EXPECT_TRUE(listeners.empty());
}

TEST(ListenersSharedPtrTest, ContainsReturnsTrueForAddedIdAndFalseForUnknownId) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListeners listeners;
    const auto id = listeners.add(impl);
    EXPECT_TRUE(listeners.contains(id));
    EXPECT_FALSE(listeners.contains(id + 1));
}

TEST(ListenersSharedPtrTest, RemoveReturnsTrueAndErasesListener) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListeners listeners;
    const auto id = listeners.add(impl);
    EXPECT_TRUE(listeners.remove(id));
    EXPECT_TRUE(listeners.empty());
}

TEST(ListenersSharedPtrTest, RemoveUnknownIdReturnsFalse) {
    SharedListeners listeners;
    EXPECT_FALSE(listeners.remove(7ULL));
}

TEST(ListenersSharedPtrTest, WasRemovedLastTrueOnlyWhenContainerBecomesEmpty) {
    auto implA = std::make_shared<TestListenerImpl>();
    auto implB = std::make_shared<TestListenerImpl>();
    SharedListeners listeners;
    const auto id1 = listeners.add(implA);
    const auto id2 = listeners.add(implB);

    bool wasRemovedLast = true;
    listeners.remove(id1, &wasRemovedLast);
    EXPECT_FALSE(wasRemovedLast);

    wasRemovedLast = false;
    listeners.remove(id2, &wasRemovedLast);
    EXPECT_TRUE(wasRemovedLast);
}

TEST(ListenersSharedPtrTest, ClearReturnsTrueWhenNonEmptyThenFalseWhenAlreadyEmpty) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListeners listeners;
    listeners.add(impl);
    EXPECT_TRUE(listeners.clear());
    EXPECT_FALSE(listeners.clear());
}

TEST(ListenersSharedPtrTest, EmptySizeAndOperatorBoolReflectContents) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListeners listeners;
    EXPECT_TRUE(listeners.empty());
    listeners.add(impl);
    EXPECT_FALSE(listeners.empty());
    EXPECT_EQ(1u, listeners.size());
    EXPECT_TRUE(static_cast<bool>(listeners));
}

TEST(ListenersSharedPtrTest, WasAdded1stTrueAgainAfterContainerEmptiedAndRefilled) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListeners listeners;
    bool wasAdded1st = false;
    const auto id = listeners.add(impl, &wasAdded1st);
    EXPECT_TRUE(wasAdded1st);
    listeners.remove(id);
    ASSERT_TRUE(listeners.empty());
    wasAdded1st = false;
    listeners.add(impl, &wasAdded1st);
    EXPECT_TRUE(wasAdded1st);
}

// =============================================================================
// Bricks::Listeners<std::shared_ptr<ITestListener>, false>  (ThreadSafe = false)
// =============================================================================
using SharedListenersUnsafe = Bricks::Listeners<std::shared_ptr<ITestListener>, false>;

TEST(ListenersSharedPtrUnsafeTest, AddReturnsNonZeroIdAndWasAdded1stOnFirstInsertion) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListenersUnsafe listeners;
    bool wasAdded1st = false;
    const auto id = listeners.add(impl, &wasAdded1st);
    EXPECT_NE(0ULL, id);
    EXPECT_TRUE(wasAdded1st);
}

TEST(ListenersSharedPtrUnsafeTest, RemoveReturnsTrueAndErasesListener) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListenersUnsafe listeners;
    const auto id = listeners.add(impl);
    EXPECT_TRUE(listeners.remove(id));
    EXPECT_TRUE(listeners.empty());
}

TEST(ListenersSharedPtrUnsafeTest, RemoveUnknownIdReturnsFalse) {
    SharedListenersUnsafe listeners;
    EXPECT_FALSE(listeners.remove(3ULL));
}

TEST(ListenersSharedPtrUnsafeTest, ClearReturnsTrueWhenNonEmptyThenFalseWhenAlreadyEmpty) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListenersUnsafe listeners;
    listeners.add(impl);
    EXPECT_TRUE(listeners.clear());
    EXPECT_FALSE(listeners.clear());
}

TEST(ListenersSharedPtrUnsafeTest, MoveConstructorTransfersListenersAndEmptiesSource) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListenersUnsafe original;
    original.add(impl);
    SharedListenersUnsafe moved(std::move(original));
    EXPECT_EQ(1u, moved.size());
    EXPECT_TRUE(original.empty());
}

TEST(ListenersSharedPtrUnsafeTest, MoveAssignmentTransfersListenersAndEmptiesSource) {
    auto impl = std::make_shared<TestListenerImpl>();
    SharedListenersUnsafe original;
    original.add(impl);
    SharedListenersUnsafe moved;
    moved = std::move(original);
    EXPECT_EQ(1u, moved.size());
    EXPECT_TRUE(original.empty());
}

// =============================================================================
// Bricks::Listeners<std::weak_ptr<ITestListener>>  (ThreadSafe = true)
// =============================================================================
using WeakListeners = Bricks::Listeners<std::weak_ptr<ITestListener>>;

TEST(ListenersWeakPtrTest, AddReturnsNonZeroIdAndWasAdded1stOnFirstInsertion) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListeners listeners;
    bool wasAdded1st = false;
    const auto id = listeners.add(std::weak_ptr<ITestListener>(impl), &wasAdded1st);
    EXPECT_NE(0ULL, id);
    EXPECT_TRUE(wasAdded1st);
    EXPECT_EQ(1u, listeners.size());
}

TEST(ListenersWeakPtrTest, AddSecondListenerReturnsDistinctIdAndWasAdded1stFalse) {
    auto implA = std::make_shared<TestListenerImpl>();
    auto implB = std::make_shared<TestListenerImpl>();
    WeakListeners listeners;
    const auto id1 = listeners.add(std::weak_ptr<ITestListener>(implA));
    bool wasAdded1st = false; // caller must pre-init false; add() only ever sets true
    const auto id2 = listeners.add(std::weak_ptr<ITestListener>(implB), &wasAdded1st);
    EXPECT_NE(id1, id2);
    EXPECT_FALSE(wasAdded1st);
    EXPECT_EQ(2u, listeners.size());
}

TEST(ListenersWeakPtrTest, AddReturnsZeroForDefaultEmptyWeakPtr) {
    WeakListeners listeners;
    const auto id = listeners.add(std::weak_ptr<ITestListener>{});
    EXPECT_EQ(0ULL, id);
    EXPECT_TRUE(listeners.empty());
}

TEST(ListenersWeakPtrTest, AddReturnsZeroForAlreadyExpiredWeakPtr) {
    std::weak_ptr<ITestListener> expired;
    {
        auto tmp = std::make_shared<TestListenerImpl>();
        expired = tmp;
    } // tmp destroyed -> expired.expired() == true
    WeakListeners listeners;
    const auto id = listeners.add(expired);
    EXPECT_EQ(0ULL, id);
    EXPECT_TRUE(listeners.empty());
}

TEST(ListenersWeakPtrTest, ContainsReturnsTrueForAddedIdAndFalseForUnknownId) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListeners listeners;
    const auto id = listeners.add(std::weak_ptr<ITestListener>(impl));
    EXPECT_TRUE(listeners.contains(id));
    EXPECT_FALSE(listeners.contains(id + 1));
}

TEST(ListenersWeakPtrTest, RemoveReturnsTrueAndErasesListener) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListeners listeners;
    const auto id = listeners.add(std::weak_ptr<ITestListener>(impl));
    EXPECT_TRUE(listeners.remove(id));
    EXPECT_FALSE(listeners.contains(id));
    EXPECT_TRUE(listeners.empty());
}

TEST(ListenersWeakPtrTest, RemoveUnknownIdReturnsFalse) {
    WeakListeners listeners;
    EXPECT_FALSE(listeners.remove(99ULL));
}

TEST(ListenersWeakPtrTest, WasRemovedLastTrueOnlyWhenContainerBecomesEmpty) {
    auto implA = std::make_shared<TestListenerImpl>();
    auto implB = std::make_shared<TestListenerImpl>();
    WeakListeners listeners;
    const auto id1 = listeners.add(std::weak_ptr<ITestListener>(implA));
    const auto id2 = listeners.add(std::weak_ptr<ITestListener>(implB));

    bool wasRemovedLast = true;
    listeners.remove(id1, &wasRemovedLast);
    EXPECT_FALSE(wasRemovedLast);

    wasRemovedLast = false;
    listeners.remove(id2, &wasRemovedLast);
    EXPECT_TRUE(wasRemovedLast);
}

TEST(ListenersWeakPtrTest, ClearReturnsTrueWhenNonEmptyThenFalseWhenAlreadyEmpty) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListeners listeners;
    listeners.add(std::weak_ptr<ITestListener>(impl));
    EXPECT_TRUE(listeners.clear());
    EXPECT_FALSE(listeners.clear());
}

TEST(ListenersWeakPtrTest, EmptySizeAndOperatorBoolReflectContents) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListeners listeners;
    EXPECT_TRUE(listeners.empty());
    listeners.add(std::weak_ptr<ITestListener>(impl));
    EXPECT_FALSE(listeners.empty());
    EXPECT_EQ(1u, listeners.size());
    EXPECT_TRUE(static_cast<bool>(listeners));
}

// Listeners is id-based bookkeeping, not a liveness cache: an entry whose
// referent has died is only ever removed via an explicit remove(id), it is
// never auto-purged just because the weak_ptr expired.
TEST(ListenersWeakPtrTest, ContainsRemainsTrueAfterReferentExpiresUntilExplicitRemove) {
    WeakListeners listeners;
    uint64_t id = 0ULL;
    {
        auto impl = std::make_shared<TestListenerImpl>();
        id = listeners.add(std::weak_ptr<ITestListener>(impl));
    } // impl destroyed -> the stored weak_ptr is now expired
    EXPECT_TRUE(listeners.contains(id));
    EXPECT_EQ(1u, listeners.size());
    EXPECT_TRUE(listeners.remove(id));
    EXPECT_TRUE(listeners.empty());
}

// =============================================================================
// Bricks::Listeners<std::weak_ptr<ITestListener>, false>  (ThreadSafe = false)
// =============================================================================
using WeakListenersUnsafe = Bricks::Listeners<std::weak_ptr<ITestListener>, false>;

TEST(ListenersWeakPtrUnsafeTest, AddReturnsNonZeroIdAndWasAdded1stOnFirstInsertion) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListenersUnsafe listeners;
    bool wasAdded1st = false;
    const auto id = listeners.add(std::weak_ptr<ITestListener>(impl), &wasAdded1st);
    EXPECT_NE(0ULL, id);
    EXPECT_TRUE(wasAdded1st);
}

TEST(ListenersWeakPtrUnsafeTest, AddReturnsZeroForAlreadyExpiredWeakPtr) {
    std::weak_ptr<ITestListener> expired;
    {
        auto tmp = std::make_shared<TestListenerImpl>();
        expired = tmp;
    }
    WeakListenersUnsafe listeners;
    const auto id = listeners.add(expired);
    EXPECT_EQ(0ULL, id);
    EXPECT_TRUE(listeners.empty());
}

TEST(ListenersWeakPtrUnsafeTest, RemoveReturnsTrueAndErasesListener) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListenersUnsafe listeners;
    const auto id = listeners.add(std::weak_ptr<ITestListener>(impl));
    EXPECT_TRUE(listeners.remove(id));
    EXPECT_TRUE(listeners.empty());
}

TEST(ListenersWeakPtrUnsafeTest, RemoveUnknownIdReturnsFalse) {
    WeakListenersUnsafe listeners;
    EXPECT_FALSE(listeners.remove(5ULL));
}

TEST(ListenersWeakPtrUnsafeTest, ClearReturnsTrueWhenNonEmptyThenFalseWhenAlreadyEmpty) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListenersUnsafe listeners;
    listeners.add(std::weak_ptr<ITestListener>(impl));
    EXPECT_TRUE(listeners.clear());
    EXPECT_FALSE(listeners.clear());
}

TEST(ListenersWeakPtrUnsafeTest, MoveConstructorTransfersListenersAndEmptiesSource) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListenersUnsafe original;
    original.add(std::weak_ptr<ITestListener>(impl));
    WeakListenersUnsafe moved(std::move(original));
    EXPECT_EQ(1u, moved.size());
    EXPECT_TRUE(original.empty());
}

TEST(ListenersWeakPtrUnsafeTest, MoveAssignmentTransfersListenersAndEmptiesSource) {
    auto impl = std::make_shared<TestListenerImpl>();
    WeakListenersUnsafe original;
    original.add(std::weak_ptr<ITestListener>(impl));
    WeakListenersUnsafe moved;
    moved = std::move(original);
    EXPECT_EQ(1u, moved.size());
    EXPECT_TRUE(original.empty());
}

// =============================================================================
// Concurrency smoke test (ThreadSafe = true, shared_ptr).
// Deliberately does NOT call invoke()/apply() -- see the note at the top of
// this file.
// =============================================================================
TEST(ListenersConcurrencyTest, ConcurrentAddRemoveContainsNoCrash) {
    SharedListeners listeners;
    std::atomic<bool> stop{false};

    std::thread addRemove([&]() {
        std::vector<uint64_t> ids;
        int i = 0;
        while (!stop.load(std::memory_order_relaxed)) {
            if (ids.empty() || (i % 2) == 0) {
                auto impl = std::make_shared<TestListenerImpl>();
                ids.push_back(listeners.add(impl));
            } else {
                listeners.remove(ids.back());
                ids.pop_back();
            }
            ++i;
        }
    });

    std::thread reader([&]() {
        for (int i = 0; i < 20000; ++i) {
            (void)listeners.size();
            (void)listeners.empty();
            (void)listeners.contains(static_cast<uint64_t>(i));
        }
    });

    reader.join();
    stop.store(true, std::memory_order_relaxed);
    addRemove.join();

    SUCCEED();
}
