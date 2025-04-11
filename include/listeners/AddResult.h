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
#pragma once // AddResult.h

namespace Bricks
{

/**
 * @brief Enumeration representing the result of adding a listener.
 */
enum class AddResult
{
    NullInput,   ///< Failure: The input listener was null.
    Duplicate,   ///< Failure: The listener already exists in the list.
    OkFirst,     ///< Success: The first listener was successfully added.
    Ok           ///< Success: The listener was successfully added.
};

/**
 * @brief Checks whether the `AddResult` indicates success.
 *
 * @param result The result to check.
 * @return `true` if the result is `AddResult::Ok` or `AddResult::OkFirst`, otherwise `false`.
 */
inline bool constexpr ok(AddResult result) {
    return AddResult::Ok == result || AddResult::OkFirst == result;
}

} // namespace Bricks
