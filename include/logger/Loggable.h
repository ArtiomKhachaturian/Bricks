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
/**
 * @brief A holder class for storing and sharing a logger instance.
 *
 * This class provides a base interface for components that need access to a shared
 * or unique logger instance, along with various logging utility methods.
 * Optionally it supports customizable base interfaces through variadic template arguments.
 *
 * @tparam TLoggerPointerType The type of the logger pointer (e.g., std::shared_ptr<Logger>).
 * @tparam BaseInterfaces Variadic template parameters for additional interfaces to inherit from.
 */
template <class TLoggerPointerType, class... BaseInterfaces>
class Loggable : public BaseInterfaces...
{
public:
    /**
     * @brief Returns a constant reference to the stored logger instance.
     *
     * @return A constant reference to the logger instance.
     */
    const auto& logger() const noexcept { return _logger; }

    /**
     * @brief Checks if logging is enabled for a specific severity level.
     *
     * @param severity The severity level to check.
     * @return `true` if logging is enabled for the specified severity, otherwise `false`.
     */
    bool canLog(LoggingSeverity severity) const;

    /**
     * @brief Checks if verbose logging is enabled.
     *
     * @return `true` if verbose logging is enabled, otherwise `false`.
     */
    bool canLogVerbose() const { return canLog(LoggingSeverity::Verbose); }

    /**
     * @brief Checks if informational logging is enabled.
     *
     * @return `true` if informational logging is enabled, otherwise `false`.
     */
    bool canLogInfo() const { return canLog(LoggingSeverity::Info); }

    /**
     * @brief Checks if warning logging is enabled.
     *
     * @return `true` if warning logging is enabled, otherwise `false`.
     */
    bool canLogWarning() const { return canLog(LoggingSeverity::Warning); }

    /**
     * @brief Checks if error logging is enabled.
     *
     * @return `true` if error logging is enabled, otherwise `false`.
     */
    bool canLogError() const { return canLog(LoggingSeverity::Error); }

    /**
     * @brief Logs a message with the specified severity and category.
     *
     * @param severity The severity level of the log message.
     * @param message The log message.
     * @param category The category of the log message.
     */
    void log(LoggingSeverity severity, std::string_view message, std::string_view category) const;

    /**
     * @brief Logs a message with the specified severity.
     *
     * @param severity The severity level of the log message.
     * @param message The log message.
     */
    void log(LoggingSeverity severity, std::string_view message) const;

    /**
     * @brief Logs a verbose message with a specific category.
     *
     * @param message The verbose log message.
     * @param category The category of the log message.
     */
    void logVerbose(std::string_view message, std::string_view category) const;

    /**
     * @brief Logs a verbose message.
     *
     * @param message The verbose log message.
     */
    void logVerbose(std::string_view message) const;

    /**
     * @brief Logs an informational message with a specific category.
     *
     * @param message The informational log message.
     * @param category The category of the log message.
     */
    void logInfo(std::string_view message, std::string_view category) const;

    /**
     * @brief Logs an informational message.
     *
     * @param message The informational log message.
     */
    void logInfo(std::string_view message) const;

    /**
     * @brief Logs a warning message with a specific category.
     *
     * @param message The warning log message.
     * @param category The category of the log message.
     */
    void logWarning(std::string_view message, std::string_view category) const;

    /**
     * @brief Logs a warning message.
     *
     * @param message The warning log message.
     */
    void logWarning(std::string_view message) const;

    /**
     * @brief Logs an error message with a specific category.
     *
     * @param message The error log message.
     * @param category The category of the log message.
     */
    void logError(std::string_view message, std::string_view category) const;

    /**
     * @brief Logs an error message.
     *
     * @param message The error log message.
     */
    void logError(std::string_view message) const;

protected:
    /**
     * @brief Constructs a Loggable object with the specified logger instance and optional base interfaces.
     *
     * @tparam Args Variadic template parameters for additional arguments required by the base interfaces.
     * @param logger The logger instance to be stored.
     * @param args Additional arguments for the base interfaces.
     */
    template<typename... Args>
    Loggable(TLoggerPointerType logger, Args&&... args);

    /**
     * @brief Returns the log category associated with the instance.
     *
     * Can be overridden in derived classes to specify a custom log category.
     *
     * @return A string view representing the log category.
     */
    virtual std::string_view logCategory() const { return {}; }

private:
    /// @brief The logger instance managed by this class.
    const TLoggerPointerType _logger;
};

/**
 * @brief A type alias for Loggable with a std::shared_ptr<Logger>.
 *
 * @tparam BaseInterfaces Additional interfaces to inherit from.
 */
template <class... BaseInterfaces>
using LoggableS = Loggable<std::shared_ptr<Logger>, BaseInterfaces...>;

/**
 * @brief A type alias for Loggable with a raw pointer to Logger.
 *
 * @tparam BaseInterfaces Additional interfaces to inherit from.
 */
template <class... BaseInterfaces>
using LoggableR = Loggable<Logger*, BaseInterfaces...>;

/**
 * @brief A type alias for Loggable with a std::unique_ptr<Logger>.
 *
 * @tparam BaseInterfaces Additional interfaces to inherit from.
 */
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
