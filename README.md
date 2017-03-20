# JaxBoy
A WIP GameBoy emulator written in C++

## Description

JaxBoy is a [GameBoy](https://en.wikipedia.org/wiki/Game_Boy) emulator written in C++. Currently it can only play through the bootrom, and it does not load any roms yet. It is WIP and is very inaccurate and incomplete.

## Status

JaxBoy is currently not able to emulate any commercial games. If you can get a binary of the 256-byte GameBoy bootrom and pass it in as build parameter, the emulator will diplay the graphics (no sound) of the bootrom pretty accurately.

While performance appears to be pretty good (read: scrolls through the Nintendo(R) screen in a seemingly timely manner), it has not been a priority and therefore is not yet intended to be accurate.

## Screenshots

![bootrom](screenshots/bootrom.png)

## Known game compatibility

Currently no games are compatible with JaxBoy.

## Building and running

JaxBoy uses [make](https://www.gnu.org/software/make/), so building is simple.

Currently it is intended to be built on MacOS with clang, but it is easy to change the Makefile for your system/toolchain.

The only dependency for building is [MiniFB](https://github.com/emoon/minifb). Build that project and then place libminifb.a into `libs/`.

Then, to build simply run:
```
make run
```

## License

Licensed under the [Apache 2.0 License](http://www.apache.org/licenses/LICENSE-2.0)(see [LICENSE](LICENSE)).
