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
#pragma once // Invoke.h
#include <memory>

namespace Bricks
{

// special helper for Listeners<> & Listener<>
template<class TListener, typename TResult = void>
class Invoke
{
public:
    template <class Method, typename... Args>
    static TResult make(const TListener& listener, const Method& method,
                        Args&&... args) {
        if (listener) {
            return ((*listener).*method)(std::forward<Args>(args)...);
        }
        return TResult();
    }
};

template<typename T, typename TResult>
class Invoke<std::weak_ptr<T>, TResult>
{
    using Forward = Invoke<std::shared_ptr<T>, TResult>;
public:
    template <class Method, typename... Args>
    static TResult make(const std::weak_ptr<T>& listenerRef, const Method& method,
                        Args&&... args) {
        return Forward::make(listenerRef.lock(), method, std::forward<Args>(args)...);
    }
};

} // namespace Bricks
