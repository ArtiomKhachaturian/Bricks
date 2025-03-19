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
#pragma once // LoggingSeverity.h

namespace Bricks
{

/**
 * @brief Enum class representing different levels of logging severity.
 *
 * Provides a hierarchy of logging levels to indicate the importance and type
 * of messages logged by the system.
 */
enum class LoggingSeverity
{
    /**
     * @brief Verbose: For detailed diagnostic data.
     *
     * This level is used for information that typically does not appear in
     * the normal debug log but is useful for diagnostic purposes.
     */
    Verbose,

    /**
     * @brief Info: Chatty level for general debugging information.
     *
     * Used for a variety of debug messages. This is the default level in debug builds.
     */
    Info,

    /**
     * @brief Warning: Indicates potential issues that may require investigation.
     *
     * This level is for events that might suggest an issue but do not prevent
     * normal operation.
     */
    Warning,

    /**
     * @brief Error: Represents critical issues that should not occur.
     *
     * This level is for serious problems that require immediate attention.
     */
    Error,
};

} // namespace Bricks
