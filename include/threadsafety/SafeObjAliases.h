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
#pragma once // SafeObjAliases.h
#include "SafeObj.h"
#include <memory>
#include <optional>

namespace Bricks
{
/**
 * @brief A thread-safe wrapper for managing `std::shared_ptr` objects.
 *
 * Provides safe access to `std::shared_ptr` objects in multithreaded environments
 * using the specified mutex type for synchronization.
 *
 * @tparam T The type of the object managed by the `std::shared_ptr`.
 * @tparam TMutexType The type of mutex used for synchronization (defaults to `std::recursive_mutex`).
 */
template <typename T, class TMutexType = std::recursive_mutex>
using SafeSharedPtr = SafeObj<std::shared_ptr<T>, TMutexType>;

/**
 * @brief A thread-safe wrapper for managing `std::weak_ptr` objects.
 *
 * Ensures safe access to `std::weak_ptr` objects in multithreaded environments
 * using the specified mutex type for synchronization.
 *
 * @tparam T The type of the object referenced by the `std::weak_ptr`.
 * @tparam TMutexType The type of mutex used for synchronization (defaults to `std::recursive_mutex`).
 */
template <typename T, class TMutexType = std::recursive_mutex>
using SafeWeakPtr = SafeObj<std::weak_ptr<T>, TMutexType>;

/**
 * @brief A thread-safe wrapper for managing `std::unique_ptr` objects.
 *
 * Facilitates safe access and management of `std::unique_ptr` objects in multithreaded
 * environments using the specified mutex type for synchronization.
 *
 * @tparam T The type of the object managed by the `std::unique_ptr`.
 * @tparam TMutexType The type of mutex used for synchronization (defaults to `std::recursive_mutex`).
 */
template <typename T, class TMutexType = std::recursive_mutex>
using SafeUniquePtr = SafeObj<std::unique_ptr<T>, TMutexType>;

/**
 * @brief A thread-safe wrapper for managing `std::optional` objects.
 *
 * Provides safe access to `std::optional` objects in multithreaded environments
 * using the specified mutex type for synchronization.
 *
 * @tparam T The type of the object contained within the `std::optional`.
 * @tparam TMutexType The type of mutex used for synchronization (defaults to `std::recursive_mutex`).
 */
template <typename T, class TMutexType = std::recursive_mutex>
using SafeOptional = SafeObj<std::optional<T>, TMutexType>;

} // namespace Bricks
