# Bricks

**Bricks** is a lightweight, header-only C++ utility library containing reusable components for multithreaded programming, event handling, and data encapsulation.  
These utilities are extracted from real-world usage across various projects to streamline development of reliable and maintainable systems.

## Features

- **Thread-safe primitives** for listener and object management
- **Logging interface** with customizable severity levels
- **Blob interface** for binary data abstraction
- Header-only design — easy to integrate, no linking required
- Highly flexible and extensible templates

## Components

### `Listener`

A threadsafe wrapper for managing a single listener object with optional mutex-based locking.  
Useful for event-driven or observer-style interfaces.

### `Listeners`

A threadsafe wrapper for managing a list of listener objects.  
Supports raw pointers, `std::shared_ptr`, and `std::weak_ptr`.  
`std::unique_ptr` is not supported by design.

### `Logger`

A minimal logger interface supporting multiple severity levels:
- Verbose
- Info
- Warning
- Error

Intended to be subclassed and customized based on the output destination.

### `SafeObj`

Encapsulates an object with optional thread safety using customizable mutex types and locking traits.  
Designed for high-performance, multithreaded environments with fine control over synchronization.

### `Blob`

An abstract base class representing a block of binary data.  
Provides size and raw pointer access. Can be extended for specific use cases like memory views or buffers.

## License

This project is licensed under the Apache License 2.0.  
See the [LICENSE](LICENSE) file for more details.

## Author

[Artiom Khachaturian](https://github.com/ArtiomKhachaturian)
