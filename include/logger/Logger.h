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
#pragma once // LogsReceiver.h
#include "LoggingSeverity.h"
#include <string_view>

namespace Bricks
{

/**
 * @brief A simple logger interface for logging messages with varying levels of severity.
 *
 * This interface provides methods to log messages at different severity levels
 * (Verbose, Info, Warning, and Error). Depending on the target purpose,
 * thread-safe implementations may be required.
 */
class Logger
{
public:
    /**
     * @brief Virtual destructor.
     *
     * Ensures proper cleanup of derived class objects when deleted through a base class pointer.
     */
    virtual ~Logger() = default;

    /**
     * @brief Checks if logging is enabled for a specific severity level.
     *
     * This method can be overridden in derived classes to enable or disable logging
     * dynamically for different severity levels.
     *
     * @param severity The severity level to check.
     * @return `true` if logging is enabled for the specified severity, otherwise `false`.
     */
    virtual bool canLog(LoggingSeverity /*severity*/) const { return true; }

    /**
     * @brief Logs a message with the specified severity and category.
     *
     * This is a pure virtual function that must be implemented by derived classes.
     *
     * @param severity The severity level of the log message.
     * @param message The log message to be recorded.
     * @param category (Optional) The category of the log message (default is empty).
     */
    virtual void log(LoggingSeverity severity, std::string_view message,
                     std::string_view category = {}) = 0;

    /**
     * @brief Logs a verbose message.
     *
     * @param message The log message to be recorded.
     * @param category (Optional) The category of the log message (default is empty).
     */
    void logVerbose(std::string_view message, std::string_view category = {}) {
        log(LoggingSeverity::Verbose, message, category);
    }

    /**
     * @brief Logs an informational message.
     *
     * @param message The log message to be recorded.
     * @param category (Optional) The category of the log message (default is empty).
     */
    void logInfo(std::string_view message, std::string_view category = {}) {
        log(LoggingSeverity::Info, message, category);
    }

    /**
     * @brief Logs a warning message.
     *
     * @param message The log message to be recorded.
     * @param category (Optional) The category of the log message (default is empty).
     */
    void logWarning(std::string_view message, std::string_view category = {}) {
        log(LoggingSeverity::Warning, message, category);
    }

    /**
     * @brief Logs an error message.
     *
     * @param message The log message to be recorded.
     * @param category (Optional) The category of the log message (default is empty).
     */
    void logError(std::string_view message, std::string_view category = {}) {
        log(LoggingSeverity::Error, message, category);
    }

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
};

} // namespace Bricks
