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

#include "SDLContext.h"
#include "core/GameBoy.h"

#include "common/Types.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>


int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        std::cerr << "Insufficient arguments!\n";
        return -1;
    }

    std::string rom_path (argv[1]);
    std::string bootrom_path (argv[2]);

    if(rom_path.substr(rom_path.length() - 3) != ".gb")
    {
        // not a DMG rom
        std::cerr << "Not a GameBoy rom!\n";
        return -1;
    }
    if(bootrom_path.substr(bootrom_path.length() - 4) != ".bin")
    {
        // must end in .bin
        std::cerr << "boot ROM must end in .bin!\n";
        return -1;
    }

    std::vector<u8> rom;
    std::vector<u8> bootrom;

    // read ROM
    std::ifstream rom_file (rom_path, std::ios::binary | std::ios::ate);
    if(rom_file.good())
    {
        rom_file.seekg(0, std::ios_base::end);
        const int rom_size = rom_file.tellg();
        rom_file.seekg(0, std::ios_base::beg);
        // allocate rom_size bytes for the rom vector
        rom.resize(rom_size);
        rom_file.read(reinterpret_cast<char*>(rom.data()), rom_size);
        rom_file.close();
    }
    else
    {
        std::cerr << "Error opening ROM!\n";
        return -1;
    }

    // read bootrom
    std::ifstream bootrom_file (bootrom_path, std::ios::binary | std::ios::ate);
    if(bootrom_file.good())
    {
        bootrom_file.seekg(0, std::ios_base::end);
        const int bootrom_size = bootrom_file.tellg();
        bootrom_file.seekg(0, std::ios_base::beg);
        // allocate bootrom_size bytes for the bootrom vector
        bootrom.resize(bootrom_size);
        if(bootrom_size != 0x100)
        {
            std::cerr << "boot ROM is not 256 bytes!\n";
            return -1;
        }
        bootrom_file.read(reinterpret_cast<char*>(bootrom.data()), bootrom_size);
        bootrom_file.close();
    }
    else
    {
        std::cout << "Error opening boot ROM!\n";
        return -1;
    }

    // Setup system options
    Core::GameBoy::Options options;

    if(argc > 3)
    {
        // parse options
        for(int i = 3; i < argc; i++)
        {
            std::string arg (argv[i]);
            try {
                ///////////////////////
                // -debug
                ///////////////////////
                if(arg == "-debug") {
                    options.isDebug = true;
                }
                ///////////////////////
                // -scale=<int>
                ///////////////////////
                else if(arg.substr(0, 6) == "-scale") {
                    // Make sure you're passing an integer to it
                    if(arg.length() < 7)
                        throw std::invalid_argument("Usage:\n-scale=<int>");
                    // Read the integer for the scale
                    // throws std::invalid_argument
                    options.scale = std::stoi(arg.substr(7));
                }
                ///////////////////////
                // INVALID ARG
                ///////////////////////
                else {
                    throw std::invalid_argument("Unknown argument " + arg);
                }
            }
            catch(std::exception& e) {
                std::cerr << "Error parsing argument: " << argv[i] << ": " << e.what() << std::endl;
                return -1;
            }
        }
    }

    int width = 160;
    int height = 144;
    // Create the system instance
    Core::GameBoy* gameboy = ( new Core::GameBoy(options, width, height, rom, bootrom) );
    // Initalize Render Context
    FrontEnd::SDLContext* sdl_context = new FrontEnd::SDLContext(width, height, options.scale, gameboy);

    // TODO: Proper mutexes
    bool update_frame = true;
    bool poll_events = true;
    // Start the SDL thread
    std::thread sdl_thread([gameboy, sdl_context, &update_frame, &poll_events](){
        while(!gameboy->IsStopped() && !sdl_context->IsStopped())
        {
            if(update_frame) {
                sdl_context->Update(gameboy->GetPPU()->GetBackBuffer());
                update_frame = false;
                poll_events = true;
            }
        }
    });
    // Start the main thread
    {
        while(!gameboy->IsStopped() && !sdl_context->IsStopped())
        {
            gameboy->Cycle();
            update_frame = true;

            if(poll_events) {
                sdl_context->PollEvents(gameboy);
                poll_events = false;
            }
        }
    }

    // Ensure both threads don't delete
    sdl_thread.join();
    if(gameboy)
        delete gameboy;
    if(sdl_context) {
        sdl_context->Destroy();
        delete sdl_context;
    }

    // After closing
    std::cout << "\n\nExiting JaxBoy...\n\n";

    return 0;
}
