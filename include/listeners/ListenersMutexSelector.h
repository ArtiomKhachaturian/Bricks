// Copyright 2025 Artiom Khachaturian
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once // ListenersMutexSelector.h
#include "SafeObj.h"

namespace Bricks
{

/**
 * @brief A selector structure to choose the appropriate mutex type based on thread-safety requirements.
 *
 * The `ListenersMutexSelector` provides a `MutexType` definition that resolves to either
 * a real mutex (`std::recursive_mutex`) or a stub mutex (`StubMutex`), depending on the
 * `ThreadSafe` boolean template parameter.
 *
 * @tparam ThreadSafe If true, selects a real mutex (`std::recursive_mutex`). If false, selects a stub mutex.
 */
template <bool ThreadSafe>
struct ListenersMutexSelector
{
    /// @brief Defines the mutex type as `std::recursive_mutex` for thread-safe operations.
    using MutexType = std::recursive_mutex;
};

/**
 * @brief Specialization of `ListenersMutexSelector` for non-thread-safe usage.
 *
 * When `ThreadSafe` is false, the `MutexType` is defined as `StubMutex`, which provides
 * dummy locking functionality to mimic mutex operations without actual synchronization.
 */
template <>
struct ListenersMutexSelector<false>
{
    /**
     * @brief A stub mutex class for non-thread-safe operations.
     *
     * The `StubMutex` class provides no-op implementations of `lock`, `try_lock`, and `unlock`,
     * allowing it to be used in non-thread-safe contexts where synchronization is unnecessary.
     */
    class StubMutex
    {
    public:
        /// @brief Default constructor.
        StubMutex() = default;

        /// @brief No-op method to mimic locking.
        void lock() {}

        /**
         * @brief No-op method to mimic a try-lock operation.
         *
         * Always returns true, indicating the lock was successful.
         *
         * @return `true`, as no actual locking occurs.
         */
        bool try_lock() { return true; }

        /// @brief No-op method to mimic unlocking.
        void unlock() {}
    };

    /// @brief Defines the mutex type as `StubMutex` for non-thread-safe operations.
    using MutexType = StubMutex;
};

} // namespace Bricks
