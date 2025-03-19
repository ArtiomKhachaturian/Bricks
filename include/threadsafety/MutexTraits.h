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
#pragma once // MutexTraits.h
#include "SharedLockGuard.h"
#include <mutex>
#include <shared_mutex>

namespace Bricks
{

/**
 * @brief A traits structure defining the types of locks for a given mutex type.
 *
 * MutexTraits provides alias types for write and read locks based on the specified mutex type.
 * This allows for flexible handling of lock types in multithreaded environments.
 *
 * @tparam TMutexType The type of mutex for which lock types are defined (e.g., std::mutex).
 */
template<class TMutexType>
struct MutexTraits
{
    /// @brief Alias for the type of write lock used with the mutex.
    using WriteLock = std::lock_guard<TMutexType>;

    /// @brief Alias for the type of read lock, defaulting to the same as the write lock.
    using ReadLock = WriteLock;
};

/**
 * @brief Specialized traits structure for std::shared_mutex.
 *
 * This specialization of MutexTraits provides appropriate alias types for shared and exclusive locks.
 * Shared locks use SharedLockGuard to manage shared access, while exclusive locks use std::lock_guard.
 */
template<>
struct MutexTraits<std::shared_mutex>
{
    /// @brief Alias for the type of write lock used with std::shared_mutex.
    using WriteLock = std::lock_guard<std::shared_mutex>;

    /// @brief Alias for the type of read lock, which allows shared access to the mutex.
    using ReadLock = SharedLockGuard<std::shared_mutex>;
};

} // namespace Bricks
