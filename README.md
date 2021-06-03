# csharp-cpp-arrow-interop-example
Very minimal example showing interop between C++ and Arrow

The C++ library builds with cmake and expects you to have already built
Arrow C++ libs installed somewhere that cmake can find them.  The library will build as a shared library.
This library must be on the path for C# to load it.

I've only tested it on Ubuntu and suspect it would not work (without minor changes) on Windows (due to
the hardcoded `.so` in `DllImport`).
