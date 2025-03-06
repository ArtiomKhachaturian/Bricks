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

// simple logger interface,
// may thread-safe implementation required (depends on target's purposes)
class Logger
{
public:
    virtual ~Logger() = default;
    virtual bool canLog(LoggingSeverity /*severity*/) const { return true; }
    virtual void log(LoggingSeverity severity, std::string_view message,
                     std::string_view category = {}) = 0;
    void logVerbose(std::string_view message, std::string_view category = {}) {
        log(LoggingSeverity::Verbose, message, category);
    }
    void logInfo(std::string_view message, std::string_view category = {}) {
        log(LoggingSeverity::Info, message, category);
    }
    void logWarning(std::string_view message, std::string_view category = {}) {
        log(LoggingSeverity::Warning, message, category);
    }
    void logError(std::string_view message, std::string_view category = {}) {
        log(LoggingSeverity::Error, message, category);
    }
    bool canLogVerbose() const { return canLog(LoggingSeverity::Verbose); }
    bool canLogInfo() const { return canLog(LoggingSeverity::Info); }
    bool canLogWarning() const { return canLog(LoggingSeverity::Warning); }
    bool canLogError() const { return canLog(LoggingSeverity::Error); }
};
