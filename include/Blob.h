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

namespace Bricks
{

class Blob
{
public:
    Blob() = default;
    virtual ~Blob() = default;
    virtual size_t size() const noexcept { return 0U; }
    virtual const uint8_t* data() const noexcept { return nullptr; }
    bool empty() const noexcept { return 0U == size() || nullptr == data(); }
    explicit operator bool () const noexcept { return size() > 0U && nullptr != data(); }
};

} // namespace Bricks
