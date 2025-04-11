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
#pragma once // Blob.h
#include <cstdint>
#include <cstddef>

namespace Bricks
{

/**
 * @brief Represents a basic, virtualized blob of data.
 *
 * The `Blob` class provides an interface for working with a block of data,
 * including its size and access to the raw data pointer. This is a base class
 * that can be extended for specific blob implementations.
 */
class Blob
{
public:
    /**
     * @brief Default constructor.
     *
     * Constructs an empty `Blob` instance with default values.
     */
    constexpr Blob() = default;

    /**
     * @brief Virtual destructor.
     *
     * Ensures proper cleanup in derived classes.
     */
    virtual ~Blob() = default;

    /**
     * @brief Gets the size of the data contained in the blob.
     *
     * This method should be overridden in derived classes to return the actual
     * size of the data. By default, it returns 0.
     *
     * @return The size of the blob's data in bytes.
     */
    virtual size_t size() const { return 0U; }

    /**
     * @brief Gets a pointer to the raw data contained in the blob.
     *
     * This method should be overridden in derived classes to return a pointer
     * to the actual data. By default, it returns `nullptr`.
     *
     * @return A constant pointer to the blob's data, or `nullptr` if no data is available.
     */
    virtual const uint8_t* data() const { return nullptr; }

    /**
     * @brief Checks if the blob is empty.
     *
     * A blob is considered empty if its size is 0 or its data pointer is `nullptr`.
     *
     * @return `true` if the blob is empty, otherwise `false`.
     */
    bool empty() const { return 0U == size() || nullptr == data(); }

    /**
     * @brief Checks if the blob is valid (non-empty).
     *
     * This operator allows the `Blob` to be used in a boolean context. A blob is
     * considered valid if its size is greater than 0 and its data pointer is not `nullptr`.
     *
     * @return `true` if the blob is valid, otherwise `false`.
     */
    explicit operator bool () const { return size() > 0U && nullptr != data(); }
};

} // namespace Bricks
