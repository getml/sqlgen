# `sqlgen::Varchar`

The `sqlgen::Varchar` class provides a type-safe way to handle SQL VARCHAR fields in C++. It enforces length checks at runtime.

## Usage

### Basic Construction

Create a VARCHAR field with a specific maximum length:

```cpp
 // Create a VARCHAR(50) field
sqlgen::Varchar<50> name;

// Initialize with a string
sqlgen::Varchar<50> name("John Doe");

// Initialize with a std::string
std::string str = "Jane Doe";
sqlgen::Varchar<50> name(str);
```

### Safe Construction with Result

Use the `make` function for safe construction with error handling:

```cpp
// Safe construction with error handling
auto result = sqlgen::Varchar<50>::make("John Doe");
if (result) {
    auto name = result.value();
    // Use name...
} else {
    // Handle error...
}
```

### Assignment and Conversion

Assign values and convert between different VARCHAR sizes:

```cpp
struct Person{
    sqlgen::Varchar<50> name;
};

person.name = "John Doe";  // Direct assignment
person.name = std::string("Jane Doe");  // From std::string

// Convert between different sizes
sqlgen::Varchar<100> long_name;
sqlgen::Varchar<50> short_name = long_name;  // Will check size at runtime
```

### Accessing Values

Access the underlying string value:

```cpp
struct Person{
    sqlgen::Varchar<50> name;
};

const auto person = Person{.name = "John Doe"};

// Get the value
const std::string& value1 = person.name();  
const std::string& value2 = person.name.get();
const std::string& value3 = person.name.value();  

// Get the maximum size
constexpr size_t max_size = name.size();
```

## Notes

- The template parameter `_size` specifies the maximum length of the VARCHAR field
- All string operations are checked against the maximum length
- The class provides both compile-time and runtime safety
- The `Result` type is used for safe construction and error handling
- The class supports:
  - Direct string assignment
  - Conversion between different VARCHAR sizes
  - Reflection for SQL operations
  - Move and copy semantics 
