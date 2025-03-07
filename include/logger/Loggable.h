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
#pragma once // Loggable.h
#include "Logger.h"
#include <memory>

namespace Bricks
{
// holder for store & share logger instance
template <class TLoggerPointerType, class... BaseInterfaces>
class Loggable : public BaseInterfaces...
{
public:
    const auto& logger() const noexcept { return _logger; }
    bool canLog(LoggingSeverity severity) const;
    bool canLogVerbose() const { return canLog(LoggingSeverity::Verbose); }
    bool canLogInfo() const { return canLog(LoggingSeverity::Info); }
    bool canLogWarning() const { return canLog(LoggingSeverity::Warning); }
    bool canLogError() const { return canLog(LoggingSeverity::Error); }
    void log(LoggingSeverity severity, std::string_view message, std::string_view category) const;
    void log(LoggingSeverity severity, std::string_view message) const;
    void logVerbose(std::string_view message, std::string_view category) const;
    void logVerbose(std::string_view message) const;
    void logInfo(std::string_view message, std::string_view category) const;
    void logInfo(std::string_view message) const;
    void logWarning(std::string_view message, std::string_view category) const;
    void logWarning(std::string_view message) const;
    void logError(std::string_view message, std::string_view category) const;
    void logError(std::string_view message) const;
protected:
    template<typename... Args>
    Loggable(TLoggerPointerType logger, Args&&... args);
    virtual std::string_view logCategory() const { return {}; }
private:
    const TLoggerPointerType _logger;
};

template <class... BaseInterfaces>
using LoggableS = Loggable<std::shared_ptr<Logger>, BaseInterfaces...>;

template <class... BaseInterfaces>
using LoggableR = Loggable<Logger*, BaseInterfaces...>;

template <class... BaseInterfaces>
using LoggableU = Loggable<std::unique_ptr<Logger>, BaseInterfaces...>;

template <class TLoggerPointerType, class... BaseInterfaces>
template<typename... Args>
inline Loggable<TLoggerPointerType, BaseInterfaces...>::
    Loggable(TLoggerPointerType logger, Args&&... args)
        : BaseInterfaces(std::forward<Args>(args)...)...
        , _logger(std::move(logger))
{
}

template <class TLoggerPointerType, class... BaseInterfaces>
inline bool Loggable<TLoggerPointerType, BaseInterfaces...>::
    canLog(LoggingSeverity severity) const
{
    return _logger && _logger->canLog(severity);
}

template <class TLoggerPointerType, class... BaseInterfaces>
inline void Loggable<TLoggerPointerType, BaseInterfaces...>::
    log(LoggingSeverity severity, std::string_view message, std::string_view category) const
{
    if (_logger) {
        _logger->log(severity, message, category);
    }
}

template <class TLoggerPointerType, class... BaseInterfaces>
inline void Loggable<TLoggerPointerType, BaseInterfaces...>::
    log(LoggingSeverity severity, std::string_view message) const
{
    if (_logger) {
        _logger->log(severity, message, logCategory());
    }
}

template <class TLoggerPointerType, class... BaseInterfaces>
inline void Loggable<TLoggerPointerType, BaseInterfaces...>::
    logVerbose(std::string_view message, std::string_view category) const
{
    log(LoggingSeverity::Verbose, message, category);
}

template <class TLoggerPointerType, class... BaseInterfaces>
inline void Loggable<TLoggerPointerType, BaseInterfaces...>::
    logVerbose(std::string_view message) const
{
    log(LoggingSeverity::Verbose, message);
}

template <class TLoggerPointerType, class... BaseInterfaces>
inline void Loggable<TLoggerPointerType, BaseInterfaces...>::
    logInfo(std::string_view message, std::string_view category) const
{
    log(LoggingSeverity::Info, message, category);
}

template <class TLoggerPointerType, class... BaseInterfaces>
inline void Loggable<TLoggerPointerType, BaseInterfaces...>::
    logInfo(std::string_view message) const
{
    log(LoggingSeverity::Info, message);
}

template <class TLoggerPointerType, class... BaseInterfaces>
inline void Loggable<TLoggerPointerType, BaseInterfaces...>::
    logWarning(std::string_view message, std::string_view category) const
{
    log(LoggingSeverity::Warning, message, category);
}

template <class TLoggerPointerType, class... BaseInterfaces>
inline void Loggable<TLoggerPointerType, BaseInterfaces...>::
    logWarning(std::string_view message) const
{
    log(LoggingSeverity::Warning, message);
}

template <class TLoggerPointerType, class... BaseInterfaces>
inline void Loggable<TLoggerPointerType, BaseInterfaces...>::
    logError(std::string_view message, std::string_view category) const
{
    log(LoggingSeverity::Error, message, category);
}

template <class TLoggerPointerType, class... BaseInterfaces>
inline void Loggable<TLoggerPointerType, BaseInterfaces...>::
    logError(std::string_view message) const
{
    log(LoggingSeverity::Error, message);
}

} // namespace Bricks
