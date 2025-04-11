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
#pragma once // RemoveResult.h

namespace Bricks
{

/**
 * @brief Enumeration representing the result of removing a listener.
 */
enum class RemoveResult
{
    NullInput,   ///< Failure: Invalid input or null pointer.
    OkLast,      ///< Success: The last listener was successfully removed.
    Ok           ///< Success: The listener was successfully removed.
};

/**
 * @brief Checks whether the `RemoveResult` indicates success.
 *
 * @param result The result to check.
 * @return `true` if the result is `RemoveResult::Ok` or `RemoveResult::OkLast`, otherwise `false`.
 */
inline bool constexpr ok(RemoveResult result) {
    return RemoveResult::Ok == result || RemoveResult::OkLast == result;
}

} // namespace Bricks
