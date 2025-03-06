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
template<class TListener>
class Invoke
{
public:
    template <class Method, typename... Args>
    static void make(const TListener& listener, const Method& method,
                     Args&&... args);
};

template<typename T>
class Invoke<std::weak_ptr<T>>
{
public:
    template <class Method, typename... Args>
    static void make(const std::weak_ptr<T>& listenerRef, const Method& method,
                     Args&&... args);
};

template<class TListener>
template <class Method, typename... Args>
inline void Invoke<TListener>::make(const TListener& listener,
                                    const Method& method,
                                    Args&&... args)
{
    if (listener) {
        ((*listener).*method)(std::forward<Args>(args)...);
    }
}

template<typename T>
template <class Method, typename... Args>
inline void Invoke<std::weak_ptr<T>>::make(const std::weak_ptr<T>& listenerRef,
                                           const Method& method,
                                           Args&&... args)
{
    Invoke<std::shared_ptr<T>>::make(listenerRef.lock(), method,
                                     std::forward<Args>(args)...);
}

} // namespace Bricks
