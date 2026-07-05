🐝 Bee Programming Language

«A lightweight interpreted programming language written in C++ for learning, experimentation, and rapid scripting.»

About

Bee is an experimental programming language created from scratch with a custom lexer, parser, and interpreter. The main goal of this project is to explore how programming languages work internally while keeping the syntax simple and readable.

Bee is still under active development, and new features are added regularly.

---

Features

- Custom Lexer
- Custom Parser
- Tree-walk Interpreter
- Variables ("int", "float", "bool", "str")
- Lists (including nested lists)
- Structs
- User-defined functions
- While loops
- If / Else statements
- Arithmetic and logical operators
- Built-in math functions
- String utilities
- List utilities
- Type conversion functions
- File handling
- Terminal colors
- Beep support
- Cross-platform codebase (Windows, Linux, Android)

---

Example

struct Person{
    int age
    str name
    bool alive
}

Person p={}

p.age=20
p.name="Ali"
p.alive=true

outl p.name
outl p.age

---

Another Example

str text="Hello Bee"

outl upper(text)
outl reverse(text)
outl len(text)

---

Project Goals

Bee is designed as a learning project to better understand:

- Language design
- Lexical analysis
- Parsing
- Interpreters
- Runtime systems
- Data structures
- Programming language implementation

Rather than competing with existing languages, Bee focuses on education, experimentation, and continuous improvement.

---

Current Built-in Features

Math

- abs
- sqrt
- pow
- floor
- ceil
- round
- sin
- cos
- tan
- log
- log10
- exp
- min
- max
- rand
- randint
- randfloat

Strings

- len
- upper
- lower
- trim
- ltrim
- rtrim
- left
- right
- mid
- reverse
- repeat
- replace
- startswith
- endswith
- contains
- find
- count
- capitalize
- slice
- split
- join

Lists

- push
- pop
- insert
- remove
- clear
- size
- index
- count
- reverse_list
- copy

Files

- read
- write
- append
- exists
- remove_file
- rename_file

Type Conversion

- Int
- Float
- Bool
- Str

Console

- color
- cls
- beep

---

Planned Features

- Classes
- Inheritance
- Modules
- Package system
- Better error messages
- Garbage collection
- Bytecode interpreter
- Virtual machine
- SDL2 graphics
- Networking
- Multithreading

---

Building

Compile the project using a C++17 compatible compiler.

Example using g++:

g++ main.cpp -std=c++17 -O2 -o bee

---

Why Bee?

Bee was created to answer one simple question:

«"How far can I build my own programming language from scratch?"»

Every part of the language—including the lexer, parser, runtime, and interpreter—is implemented manually to better understand the fundamentals of compiler and interpreter development.

---

Contributing

Suggestions, bug reports, and pull requests are always welcome.

If you have ideas for improving Bee, feel free to open an issue or submit a pull request.

---

License

This project is open source.

Choose any license that fits your goals (MIT is recommended).

---

Author

Created with ❤️ by Abolfazl Mohammadi.

🐝 Happy Coding with Bee!
