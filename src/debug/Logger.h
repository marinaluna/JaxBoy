// Copyright (C) 2017 Ryan Terry
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "../common/Types.h"
#include "../core/memory/MemoryBus.h"

#include <memory>
#include <string>

namespace Core {
    class Processor;
}; // namespace Core

namespace Debug {
namespace Logger {
    // generic log
    void Log(const std::string prefix, const std::string& msg);
    // Log processor register states
    void LogRegisters(const Core::Processor& processor);
    // Log IO Registers in memory
    void LogIORegisters(std::shared_ptr<Memory::MemoryBus>& memory_bus);
    // Log a chunk of memory specified by the caller
    void LogMemory(std::shared_ptr<Memory::MemoryBus>& memory_bus,
                   u16 address, u16 bytes);
    // Disassembles instructions and logs the result
    void LogDisassembly(std::shared_ptr<Memory::MemoryBus>& memory_bus,
                        u16 address, u16 bytes);
}; // namespace Logger
}; // namespace Debug

// Quick macros for messages
#define LOG_MSG(msg)    \
    Debug::Logger::Log("INFO: ", msg);
#define LOG_WARN(msg)   \
    Debug::Logger::Log("WARN: ", msg);
#define LOG_ERROR(msg)  \
    Debug::Logger::Log("ERROR: ", msg);
#define LOG_VRAM(memory_bus)                          \
    LOG_MSG("VRAM: 8000h - BFFFh\n");   \
    LogMemory(memory_bus, 0x8000, 0x4000);
