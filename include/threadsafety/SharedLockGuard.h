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
#pragma once // SharedLockGuard.h
#include <mutex>

namespace Bricks
{

/**
 * @brief A utility class that manages a shared lock on a shared mutex.
 *
 * SharedLockGuard provides RAII-style locking functionality for shared mutexes.
 * It ensures that the mutex is locked upon creation and automatically unlocked
 * upon destruction. This is specifically designed for use with shared locking
 * in multithreaded environments.
 *
 * @tparam TSharedMutex The type of the shared mutex (e.g., std::shared_mutex).
 */
template <class TSharedMutex>
class SharedLockGuard
{
public:
    /**
     * @brief Constructs a SharedLockGuard and locks the shared mutex.
     *
     * @param mutex The shared mutex to be locked.
     */
    SharedLockGuard(TSharedMutex& mutex);

    /**
     * @brief Constructs a SharedLockGuard, assuming the shared mutex is already locked.
     *
     * This constructor does not lock the mutex, instead it adopts the existing lock.
     *
     * @param mutex The shared mutex that is already locked.
     * @param adopt_lock A tag indicating that the lock is adopted (use std::adopt_lock_t).
     */
    SharedLockGuard(TSharedMutex& mutex, std::adopt_lock_t);

    /**
     * @brief Destructs the SharedLockGuard and unlocks the shared mutex.
     */
    ~SharedLockGuard() { _mutex.unlock_shared(); }

private:
    /// @brief Copy construction is disabled to prevent copying.
    SharedLockGuard(const SharedLockGuard&) = delete;

    /// @brief Copy assignment is disabled to prevent copying.
    SharedLockGuard& operator=(const SharedLockGuard&) = delete;

private:
    /// @brief Reference to the shared mutex managed by this guard.
    TSharedMutex& _mutex;
};

template <class TSharedMutex>
inline SharedLockGuard<TSharedMutex>::SharedLockGuard(TSharedMutex& mutex)
    : _mutex(mutex)
{
    _mutex.lock_shared();
}

template <class TSharedMutex>
inline SharedLockGuard<TSharedMutex>::SharedLockGuard(TSharedMutex& mutex, std::adopt_lock_t)
    : _mutex(mutex)
{
}

} // namespace Bricks
