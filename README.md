# the_Foundation: a C11 library

An object-oriented C library whose API is designed for a particular coding style, taking cues from C++ iterators, STL collections, and Qt.

## Introduction

I used to write a lot of C++. Over the years, code bases grew larger and more complex, and despite advances in hardware capabilities, everything got slower and more cumbersome. There is a real cost incurred by an ever-increasing cognitive load. C++ seems to be evolving toward being an all-purposes language, so it includes all the bells and whistles in addition to the kitchen sink, but personally, I like something that is smaller and optimized around a small set of core ideas.

These core ideas can be summarized as:

* Systematic naming convention that is supported and enforced via macros.
* Bare-bones object/class system.
* Memory management assistance: a manually operated garbage collector, and reference counting for all instances of a class.
* Fluent interfaces where one can chain method calls without losing track of memory ownership or parameter types.
* Coherent iterator system for all collection types.

### So, it uses macros... yikes?

Yes, preprocessor macros are a key part of the library. They are used for all the boilerplate declarations needed to make the rest of the system work in a coherent manner. While one could write all of it out manually, that would a) take a lot more effort, b) be a maintenance nightmare, and c) be quite error-prone.

Here is an example of declaring a simple data type that uses a constructor and a destructor.

    iDeclareType(Example)
    iDeclareTypeConstruction(Example)

These would give you an opaque type `iExample` and declare its basic initializer and deinitializer methods:

    void        init_Example        (iExample *);
    void        deinit_Example      (iExample *);

These two methods one needs to implement manually for the type. They initialize the members of the type as needed, allocate resources, etc. But the macros also automatically fully define the following methods as very small `static inline` functions:

    iExample *  new_Example         (void);
    void        delete_Example      (iExample *);
    iExample *  collect_Example     (iExample *);
    iExample *  collectNew_Example  (void);

Using these, one can create and destroy instances of the type without having access to the actual members of `iExample`. The collector methods are important as well: one can use those at appropriate times, for example when receiving a return value from a function, to give ownership of the instance to the garbage collector. Then the object can be used normally until the a garbage collector is told to free all the previously collected memory. In practice, this could happen periodically in a program, for example immediately after rendering a frame.

### "method_Type" naming convention

Global identifiers and functions generally use the following naming convention:

* `iType`: the prefix `i` is used for the global namespace: all types and verb-like macros. 
* `Impl_Type`: the private implementation of Type, i.e., a `struct` containing the member variables.
* `Class_Type`: the class metadata (vtable) of Type, for types that use the class mechanism.
* `method_Type`: method of a Type.
* `method_Type_`: private method of a Type; the underscore as a suffix is used for identifiers not visible outside the current source module.
* `method_Type(iType *)`: the first parameter is always the instance pointer.
* `identifier_`: a static global variable or method, i.e., member of a nameless entity (the part following the underscore is empty).

C doesn't have namespaces, so some kind of a prefix is required to avoid conflicts. `i` was chosen to be visually as small as possible, so it doesn't interfere readability as much as a longer or large capital letter prefix.

Usually type names precede a method name in C APIs, but I find it makes reading the code more difficult. The beginnings of words is important as the eye is naturally drawn to them, so we want to have actually meaning information at the beginning of each line of code.

Having the type name be the last part of an identifier is also advantegous because thanks to its location, it becomes naturally associated with the instance pointer, i.e., the first parameter. This enables chaining method calls without losing track of types.

This naming convention is also why the library is called "the\_Foundation": "Foundation" is the actual name, representing the library's role as a low-level programming framework upon which applications can be built.

### Iterators

Iterators are one of the most useful parts of C++ STL. The iterators in the\_Foundation try to replicate some of STL's most useful properties:

* Universal "for each" mechanism for every iterable type.
* Mutable and const variants.
* Forward and reverse directions.

The primary limitation for iterators is that they cannot allocate memory on the heap. The iteration must be doable with only local variables that do not need deinitialization when exiting the scope. This allows the use of a simple `for` loop:

    iArrayIterator i;    
    for (init_ArrayIterator(&i, foo); i.value; next_ArrayIterator(&i)) {
        /* do stuff... */
    }

There is a macro for the code above:

    iForEach(Array, i, foo) {
        /* do stuff... */
    }

Similarly, there are `iConstForEach`, `iReverseForEach`, and `iReverseConstForEach` macros for different types of iterators. These can be used with any type as long as the corresponding functions are available to be called; it all hinges on the naming convention.

## API and source code conventions

### General

- Global symbols like type names and macros use the `i` prefix (e.g., `iMin`).

- Method names and variables use camelCase.

- Type names and classes start with a capital letter (following the `i` prefix).

- Preprocessor macros and constants use naming similar to classes (e.g., `iDeclareType`). They begin with a verb.

- The general base class `iObject` implements reference counting. The class of an object determines how the object is deinitialized.

- In functions where an object is passed as an argument, the reference count must be incremented if the function holds a pointer to the object. Otherwise, the reference count should not be modified.

### Types and classes

All class members use the class name as a _suffix_, e.g., `length_String`. This improves readability and associates the first argument (the `d` object, equivalent to `this` in C++) with the type of the class.

A static/private member of a class additionally adds an extra underscore to the suffix, e.g., `element_Array_`.

Type names are declared with the `iDeclareType(Name)` macro. The implementation struct is always called `struct Impl_Name` that has a typedef alias called `iName`. The `Impl_Name` struct should be kept opaque (only declared in the header) by default to hide the implementation details.

`static inline` functions (or macros) are used to define member functions with default values for parameters.

### Construction and destruction

For a given type `Type`:

- `new_Type` allocates memory for a Type instance from the heap and initializes it by calling the init method `init_Type`. The macro `iMalloc(Type)` is provided for convenience.
- `delete_Type` deletes the object after deinitializing with `deinit_Type`.
- `init_Type` initializes an object's memory (e.g., zeroing it) inside a memory buffer with uninitialized contents. The memory can be located anywhere (heap or stack).
- `deinit_Type` releases any memory/resource allocations, but not delete the object itself nor is any of the object's state reset to zero. The memory contents are considered undefined afterwards.

### Iterators

- The member `value` is represents the current element. If NULL or zero, the iteration will stop.
- The `value` must be the first member in an iterator struct. Derived iterators should use an anonymous union to alias the value pointer to an appropriate type, while remaining compatible with the base class's iterator implementation.
- Iterators may have additional members depending on the type of the data and the requirements for internal state.
- Non-const iterators have a method called `remove` if the currently iterated element can be removed during iteration. Using this ensures that memory owned by the container itself will be released when the element is deleted.

## License

[BSD-2-Clause](https://opensource.org/licenses/BSD-2-Clause)

## Author

the\_Foundation has been written by Jaakko Keränen <jaakko.keranen@iki.fi>.
