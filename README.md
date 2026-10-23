# eunsortedvector

`eunsortedvector<T, Alloc>` is a distinct, header-only C++11 container backed by
`std::vector<T, Alloc>`. Include `eunsortedvector.hpp`. It provides the C++11 vector API
with bounds-checked `operator[]` and unordered erase operations. It is not
implicitly convertible to `std::vector`.

The project, container class, and example executable are named `eunsortedvector`.

## Differences from std::vector

- Both `operator[]` and `at()` call the underlying vector's `at()` and throw
  `std::out_of_range` for an index greater than or equal to `size()`.
- Erasing elements can change the order of the remaining elements. Insertion
  still uses normal `std::vector` insertion semantics.
- Comparisons remain order-sensitive: equality compares the stored sequence,
  and ordering is lexicographical, not a comparison of unordered sets.
- `front()`, `back()`, and `pop_back()` still require a nonempty container.
- APIs introduced after C++11 are not exposed; `emplace_back` returns `void`,
  even when compiled with a newer language standard.

## Unordered erase

`erase(pos)` swaps the last element into the removed position and pops the tail.
`erase(first, last)` fills the removed range with surviving tail elements and
pops the removed count. These operations do not preserve element order. They
return the iterator at the original removal index, or `end()` when no element
remains there; an empty range returns the unchanged `first` position.
Single-element removal takes constant time and range removal takes time linear
in the removed count. Erase requires valid iterators, just like `std::vector`;
single-element erase cannot receive `end()`. If an element swap throws, erase
can leave element values partially modified.

For range removal, the implementation swaps up to
`min(last - first, end() - last)` elements from the end into the start of the
removed range, preserving the relative order of those tail replacements, then
pops `last - first` elements.

```cpp
eunsortedvector<int> values{1, 2, 3, 4, 5};
auto next = values.erase(values.cbegin() + 1);
// values: {1, 5, 3, 4}; *next == 5

values = {1, 2, 3, 4, 5, 6};
next = values.erase(values.cbegin() + 1, values.cbegin() + 3);
// values: {1, 5, 6, 4}; *next == 5
```

Erase does not reallocate. Iterators and references to popped elements, and the
old past-the-end iterator, are invalidated. Those referring to surviving
positions remain valid, but positions participating in a swap can hold different
values. Other operations follow the underlying vector's invalidation rules.
Elements must support swapping when instantiated erase overloads use
`std::iter_swap`.

## API overview

All member types come from `std::vector<T, Alloc>`, including allocator, pointer,
reference, size, difference, and iterator types.

| Category | Members |
| --- | --- |
| Construction | Default, allocator, count, count/value, iterator range, copy, move, allocator-extended copy/move, initializer list |
| Assignment | Copy, move, initializer-list `operator=`; count/value, iterator-range, and initializer-list `assign` |
| Allocator | `get_allocator` |
| Element access | `at`, `operator[]`, `front`, `back`, `data` (except `bool`) |
| Iterators | `begin`, `end`, `cbegin`, `cend`, `rbegin`, `rend`, `crbegin`, `crend` |
| Capacity | `empty`, `size`, `max_size`, `reserve`, `capacity`, `shrink_to_fit` |
| Modifiers | `clear`, all C++11 `insert` overloads, `emplace`, both `erase` overloads, `push_back`, `emplace_back`, `pop_back`, both `resize` overloads, `swap` |
| Comparisons | `==`, `!=`, `<`, `<=`, `>`, `>=` between containers with the same template arguments |
| Non-member operations | ADL `swap`; `std::hash` for `eunsortedvector<bool, Alloc>` |

Allocator-aware storage, growth, and forwarded operations are handled by
`std::vector<T, Alloc>`. Move-only elements are supported where the corresponding
underlying operations permit them. For generic swapping, use `using std::swap;`
followed by `swap(left, right)` to select the non-member overload via ADL.

### bool specialization behavior

`eunsortedvector<bool>` uses the underlying packed `std::vector<bool>` storage:

- Mutable element references are bit proxies rather than `bool&`.
- `data()` is unavailable because the storage is not a contiguous `bool` array.
- `flip()` inverts all bits.
- `eunsortedvector<bool>::swap(left_reference, right_reference)` swaps two bit
  proxies.
- `std::hash<eunsortedvector<bool, Alloc>>` delegates to the underlying vector's
  hash. No hash specialization is provided for other element types.

## Usage

Add this directory to your compiler's include path and include `eunsortedvector.hpp`.
No library needs to be linked.

```cpp
#include "eunsortedvector.hpp"
#include <iostream>

int main()
{
    eunsortedvector<int> values{1, 2, 3};
    values.push_back(4);

    for (int value : values) {
        std::cout << value << '\n';
    }
}
```

## Build and test

Requirements: a C++11 compiler, CMake 3.10 or newer, and a build tool such as
Make or Ninja.

For a portable build using your default compiler:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/eunsortedvector
```

The commands above require CMake 3.20 or newer (`ctest --test-dir`). With older
CMake versions, configure from inside `build` using `cmake ..`, then build
with `cmake --build .` and run `ctest --output-on-failure` there.

Alternatively, the included machine-specific preset uses `/usr/bin/gcc` and
`/usr/bin/g++-16` and requires CMake 3.28 or newer:

```sh
cmake --preset 'GCC 16.0.1 x86_64-linux-gnu'
cmake --build 'out/build/GCC 16.0.1 x86_64-linux-gnu'
ctest --test-dir 'out/build/GCC 16.0.1 x86_64-linux-gnu' --output-on-failure
```

To skip building tests, configure with `-DBUILD_TESTING=OFF`.

Tests cover constructors, custom allocators, access, iterators, comparisons,
modifiers, unordered erase ranges and return values, move-only elements, packed
booleans, object lifetimes, and exception safety during reallocation.

Each named test prints `[PASS]` or `[FAIL]`, followed by a passed/failed summary.
Failures include the exception message; remaining tests still run, and the
executable exits with a nonzero status if any test fails. Use `ctest --verbose`
instead of `ctest --output-on-failure` to see output for passing tests too, or
run the `eunsortedvector_tests` executable directly from your build directory.

## VS Code

Open this directory as a workspace with the **CMake Tools** and **C/C++**
extensions installed.

- The CMake **Build** control is enabled in the status bar.
- **Ctrl+Shift+B** builds using the GCC preset's output directory.
- **F5**, with **Debug eunsortedvector** selected, builds and launches the executable
  using `/usr/bin/gdb`.

Configure with the included GCC preset first. The supplied task and debug
configuration use that preset's build directory, not the portable `build`
directory.

## Repository layout

- `eunsortedvector.hpp`: header-only container.
- `main.cpp`: example application.
- `tests/eunsortedvector_tests.cpp`: API, allocator, lifetime, and exception-safety tests.
- `CMakeLists.txt`: application and CTest targets.

Generated build output and executables are excluded by `.gitignore`.
Shared VS Code configuration and `CMakePresets.json` remain versionable;
machine-local overrides belong in the ignored `CMakeUserPresets.json`.
