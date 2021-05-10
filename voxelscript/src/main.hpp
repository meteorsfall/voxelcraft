
#ifdef _COMPILE_VS_NATIVE_
#include <functional>
#include <cstring>

#include <thread>
#include <chrono>
#include <time.h>
#endif

#ifdef _COMPILE_VS_NATIVE_
using std::function;
#else
using nostd::function;
#endif

[[noreturn]] void _cstr_abort(const char* message, const char* file, int start_line, int start_char, int end_line, int end_char);

struct string {
    int _size;
    const char* _data;
    int size() {
        return _size;
    }
    const char* data() {
        return _data;
    }
    string() : _data(nullptr), _size(0) {}
    string(const char* str) : _data(str), _size(__builtin_strlen(str)) {}
    string(const char* str, int size) : _data(str), _size(size) {}
    string substr(int start, int len) {
        return string(_data + start, len);
    }
    void copy(char* str, int size) {
        __builtin_memcpy(str, _data, size);
    }
    char at(int index) {
#ifndef NO_BOUNDS_CHECKING
        if (index >= _size) {
            _cstr_abort("String Index out-of-bounds", "??", 0, 0, 0, 0);
        }
#endif
        return _data[index];
    }
    int find(string str) {
        const char* s = __builtin_strstr(_data, str.data());
        if (s == nullptr) {
            return -1;
        } else {
            return (int)(s - _data);
        }
    }
};

#ifndef _COMPILE_VS_NATIVE_
[[noreturn]] void g_abort(string s) {
    panic(&s);
}
void g_print(string s) {
    print(&s);
}
#endif

struct string_buffer {
    string_buffer(int size) : _data(new char[size]), _size(size), _loc((char*)_data) {}
    ~string_buffer() {
        delete[] _data;
    }
    void write(const void* buf, int size) {
        __builtin_memcpy(_loc, buf, size);
        _loc += size;
    }
    void write(char c) {
        *_loc = c;
        _loc++;
    }
    const char* data() {
        return (char*)_data;
    }
    int size() {
        return _loc - _data;
    }
    string to_string() {
        return string(this->data(), this->size());
    }
    void flush() {
        *_loc = '\0';
    }
private:
    char* _data;
    int _size;
    char* _loc;
};

// String buffer implementations
string_buffer& operator<<(string_buffer& os, const char* str)
{
    os.write(str, __builtin_strlen(str));
    return os; 
}

template<typename T>
int write_int_to_buffer(char* buf, T integer) {
    int len = 0;
    int start = 0;
    if (integer < 0) {
        buf[len++] = '-';
        start++;
        integer = -integer;
    }
    if (integer == 0) {
        buf[len++] = '0';
    } else {
        while(integer > 0) {
            buf[len++] = '0' + (integer % 10);
            integer /= 10;
        }
    }
    for(int i = 0; i < (len-start)/2; i++) {
        char tmp = buf[start+i];
        buf[start+i] = buf[len-1-i];
        buf[len-1-i] = tmp;
    }
    return len;
}

string_buffer& operator<<(string_buffer& os, int integer)
{
    char buf[11]; // "-2147483648" is the longest string
    int len = write_int_to_buffer<int>(buf, integer);
    os.write(buf, len);
    return os; 
}
string_buffer& operator<<(string_buffer& os, long long integer)
{
    char buf[20]; // "-9223372036854775808" is the longest string
    int len = write_int_to_buffer<long long>(buf, integer);
    os.write(buf, len);
    return os; 
}
string_buffer& operator<<(string_buffer& os, bool val)
{
    if (val) {
        os.write("true", 4);
    } else {
        os.write("false", 5);
    }
    return os;
}
string_buffer& operator<<(string_buffer& os, char c)
{
    os.write(c);
    return os;
}
string_buffer& operator<<(string_buffer& os, double f_val)
{
    typedef unsigned long long uint64;
    uint64 val = *((uint64*)&f_val);

    // Check Sign Bit
    if (val & (1ULL << 63)) {
        os << '-';
        val &= ~(1ULL << 63);
    }

    // Raw Exponent
    uint64 raw_exp = val >> 52;
    // Raw Mantissa
    uint64 raw_mant = val & ((1ULL << 52) - 1);

    int exp_adjust = -1023 - 52;

    // Check for NaN/Infinity
    if (raw_exp == 0x7ff) {
        if (raw_mant == 0) {
            os << "Infinity";
            return os;
        } else {
            os << "NaN";
            return os;
        }
    }
    // Check for 0/Subnormals
    if (raw_exp == 0) {
        if (raw_mant == 0) {
            os << "0.0";
            return os;
        } else {
            // Subnormals
            // exp = 2^(1-1023), not 2^(raw_exp-1023) which would equal 2^(0-1023)
            exp_adjust++;
            // Interpret as 0.raw_mant, instead of 1.raw_mant
            // To fix this, we slide raw_mant over until it's of the form 1.raw_mant
            // TODO: Make resolution_available know how much this value has slid over,
            // so that rounding works better
            while((raw_mant >> 52) == 0) {
                raw_mant <<= 1;
                exp_adjust--;
            }
        }
    }

    // Now, we can interpret exp/mant normally
    int exp = (int)(val >> 52) + exp_adjust;
    uint64 mant = (1ULL << 52) | raw_mant;
    // Repr: mant * 2^exp

    // Convert 2^exp to 10^ten_exp_f, via ten_exp_f = exp * log(2)/log(10)
    double ten_exp_f = exp * 0.301029995663981198017467022509663365781307220458984375;
    // New Repr: mantissa * 10^ten_exp_f
    int ten_exp = (int)ten_exp_f;
    // New Repr: (mantissa * 10^(ten_exp_f - ten_exp)) 10^ten_exp
    uint64 ten_mantissa = (uint64)(mant * __builtin_pow(10, ten_exp_f - ten_exp));
    // New Repr: ten_mantisssa * 10^ten_exp

    char buf[20]; // "18446744073709551615" is the longest string
    int len = write_int_to_buffer<uint64>(buf, ten_mantissa);

    // Find out how much resolution is actually necessary
    int resolution_available = 14;
    while (buf[resolution_available] == '0') {
        resolution_available--;
    }

    // Place decimal at index 1, rather than at len
    int decimal_place = 1;
    ten_exp += len - 1;

    // If we're within 14 digits of the decimal point, just move the decimal point
    if (0 < ten_exp && ten_exp < 14) {
        decimal_place += ten_exp;
        ten_exp = 0;
    }
    // If we're within 8 digits of 0.000000, just move the decimal point
    if (-8 < ten_exp && ten_exp < 0) {
        decimal_place += ten_exp;
        ten_exp = 0;
    }

    // Handle fractions
    if (decimal_place <= 0) {
        os.write("0.", 2);
    }
    // If we're deeper into a small number, keep writing 0's
    while (decimal_place < 0) {
        os.write('0');
        decimal_place++;
    }

    int last = 0;

    if (decimal_place == 0) {
        os.write(buf, len);
        last = len-1;
    } else {
        // Write 0s up to the decimal place location
        while (len <= decimal_place) {
            buf[len++] = '0';
        }
        // Write the buf up til before the decimal_place
        os.write(buf, decimal_place);
        // Write the decimal place itself
        os.write('.');
        // Write the buf that takes place after the decimal place
        os.write(buf[decimal_place]);
        last = decimal_place;
        // If there's actually resolution to write, write it
        if (resolution_available > decimal_place + 1) {
            os.write(buf + decimal_place + 1, resolution_available - decimal_place - 1);
            last = resolution_available - 1;
        }
    }

    // TODO: Implement rounding on `last` digit

    // If there's any left-over exponent, write it now
    if (ten_exp != 0) {
        os.write('e');
        os << ten_exp;
    }

    return os; 
}

string_buffer& operator<<(string_buffer& os, string s)
{
    os.write(s.data(), s.size());
    return os; 
}

[[noreturn]] void _abort(string message, const char* file, int start_line, int start_char, int end_line, int end_char) {
    int buf_size = sizeof(char) * ((message.size() + __builtin_strlen(file) + 2) + 128);

    string_buffer out(buf_size);

    out << file << ": " << start_line << ":" << start_char << " -> " << end_line << ":" << end_char << " " << message.data() << "\n";
    out.flush();

#ifdef _COMPILE_VS_NATIVE_
    std::cout.flush();
    std::cerr << out.data();
    std::cerr.flush();
    exit(-1);
#else
    g_abort(out.to_string());
    while(true);
#endif
}

// Used by string struct, since the string struct hasn't been declared yet at the time of this call
[[noreturn]] void _cstr_abort(const char* message, const char* file, int start_line, int start_char, int end_line, int end_char) {
    _abort(string(message), file, start_line, start_char, end_line, end_char);
}

#include <initializer_list> // std::initializer_list<T>

template<typename T>
struct dynamic_array {
    dynamic_array() {}
    dynamic_array(int size) {
        resize(size);
    }
    dynamic_array(std::initializer_list<T> list) {
        reserve(list.size());
        _size = list.size();
        int i = 0;
        for(auto& a : list) {
            new(&data[i]) T(a);
            i++;
        }
    }
    ~dynamic_array() {
        resize(0);
        delete[] (char*)data;
    }
    void resize(int new_size) {
        reserve(new_size);
        // Destruct any elements if we must decrease in size
        for(int i = new_size; i < _size; i++) {
            data[i].~T();
        }
        // Default-Construct any new elements
        for(int i = _size; i < new_size; i++) {
            new(&data[i]) T;
        }
        _size = new_size;
    }
    void reserve(int new_capacity) {
        if (new_capacity > capacity) {
            if (capacity * 2 > new_capacity) {
                new_capacity = capacity * 2;
            }
            // Create new buffer
            T* new_data = (T*)(new char[new_capacity * sizeof(T)]);

            // We don't need to memset because we can leave the uninitialized data as undefined
            // They won't access it because .at() checks for size
            // __builtin_memset(new_data + capacity /* * sizeof(T) is automatic */, 0, (new_capacity - capacity) * sizeof(T));

            for(int i = 0; i < _size; i++) {
                // Construct new element from old element
                new(&new_data[i]) T(standard::move(data[i]));
                // Deconstruct dead element
                data[i].~T();
            }
            delete[] (char*)data;
            data = new_data;
            capacity = new_capacity;
        }
    }
    T& at(int index) const {
#ifndef NO_BOUNDS_CHECKING
        if (index >= _size) {
            _abort("Array Index out-of-bounds", "??", 0, 0, 0, 0);
        }
#endif
        return data[index];
    }
    T& operator[](int index) const {
        return data[index];
    }
    void push_back(T datum) {
        reserve(_size + 1);
        new(&data[_size]) T(datum);
        _size++;
    }
    int size() const {
        return _size;
    }
public:
    T* data = nullptr;
    int _size = 0;
    unsigned int reference_count = 0;
    int capacity = 0;
};

#define id_type unsigned short

// All objects inherit from this
class Object {
public:
    // Keep track of reference count for deallocations
    unsigned int reference_count = 1;
    // Keep track of object id
    const id_type object_id;

    // Initialize a new object
    Object(id_type object_id) : object_id(object_id) {};
};

template<typename T>
class ObjectRef {
public:
    T* obj;
    ObjectRef() : obj(nullptr) {}
    ObjectRef(T* obj) : obj(obj) {
#ifndef NO_REFERENCE_COUNTING
        obj->reference_count++;
#endif
    }
    // If we can implicitly convert from U* to T*,
    // then we can create a ObjectRef<T> from an ObjectRef<U>
    template<typename U>
    ObjectRef(ObjectRef<U> other) {
        // Grab reference to Object
        obj = other.obj;
#ifndef NO_REFERENCE_COUNTING
        // Nullify the other ObjectRef
        other.obj = nullptr;
#endif
    }
    ~ObjectRef()
    {
#ifndef NO_REFERENCE_COUNTING
        if (obj) {
            obj->reference_count--;
            if (obj->reference_count == 0) {
                delete obj;
            }
        }
#endif
    }
    ObjectRef(const ObjectRef<T>& other) // copy constructor
    : ObjectRef(other.obj)
    {}
    ObjectRef(ObjectRef<T>&& other) noexcept // move constructor
    {
        this->obj = other.obj;
#ifndef NO_REFERENCE_COUNTING
        other.obj = nullptr;
#endif
    }
    ObjectRef<T>& operator=(const ObjectRef<T>& other) // copy assignment
    {
        return *this = ObjectRef(other);
    }
    ObjectRef<T>& operator=(ObjectRef<T>&& other) noexcept // move assignment
    {
#ifdef NO_REFERENCE_COUNTING
        this->obj = other.obj;
#else
        standard::swap(this->obj, other.obj);
#endif
        return *this;
    }
    T& operator*() const
    {
#ifndef NO_BOUNDS_CHECKING
        if (obj == nullptr) {
            _abort("Null pointer exception", "??", 0, 0, 0, 0);
        }
#endif
        return *obj;
    }
    T* operator->() const
    {
#ifndef NO_BOUNDS_CHECKING
        if (obj == nullptr) {
            _abort("Null pointer exception", "??", 0, 0, 0, 0);
        }
#endif
        return obj;
    }
    // Define implicit conversion to T*
    operator T*() const { return obj; }
};

// Trait instance
class Trait {
public:
    ObjectRef<Object> obj;
    Trait(Object* obj) : obj(obj) {};
};

// Object Instance
typedef Object* ObjectInstance;

#define TRAIT_HEADER \
      class _Instance : public Trait { \
      public: \

#define TRAIT_MID1 \
          class _Vtable { \
          public:

#define TRAIT_MID \
          }; \
          _Instance() : Trait(nullptr) {} \
          _Instance(Object* obj) : Trait(obj) { \
          };

#define TRAIT_FOOTER \
      };

// Array of all vtables
static void* vtbls[1024][1024];

template<typename T>
bool is_class(Object* obj) {
    return obj->object_id == T::object_id;
}

template<typename T>
bool is_trait(Object* obj) {
    return vtbls[obj->object_id][T::trait_id] != nullptr;
}

template<typename T>
Object* cast_to_trait(Object* obj, const char* error_file, int error_start_line, int error_start_char, int error_end_line, int error_end_char) {
    if (!is_trait<T>(obj)) {
        _abort("Fail to cast!", error_file, error_start_line, error_start_char, error_end_line, error_end_char);
    }
    return obj;
}

template<typename T>
T* cast_to_class(Object* obj, const char* error_file, int error_start_line, int error_start_char, int error_end_line, int error_end_char) {
    if (!is_class<T>(obj)) {
        _abort("Fail to cast!", error_file, error_start_line, error_start_char, error_end_line, error_end_char);
    }
    return static_cast<T*>(obj);
}

namespace _Array_ {
  template<typename T>
  void push(dynamic_array<T>* arr, T val) {
      if (arr == nullptr) {
          _abort("Null pointer exception", "??", 0, 0, 0, 0);
      }
      arr->push_back(val);
  }
  template<typename T>
  T pop(dynamic_array<T>* arr, T val) {
      if (arr == nullptr) {
          _abort("Null pointer exception", "??", 0, 0, 0, 0);
      }
      T ret = arr->back();
      arr->pop_back();
      return ret;
  }
  template<typename T>
  T remove(dynamic_array<T>* arr, int index) {
      if (arr == nullptr) {
          _abort("Null pointer exception", "??", 0, 0, 0, 0);
      }
      T ret = arr->at(index);
      arr->erase(arr->begin() + index);
      return ret;
  }
  template<typename T>
  int size(dynamic_array<T>* arr) {
      if (arr == nullptr) {
          _abort("Null pointer exception", "??", 0, 0, 0, 0);
      }
      return arr->size();
  }
  template<typename T>
  void resize(dynamic_array<T>* arr, int size) {
      if (arr == nullptr) {
          _abort("Null pointer exception", "??", 0, 0, 0, 0);
      }
      arr->resize(size);
  }
  template<typename T>
  int get_capacity(dynamic_array<T>* arr) {
      if (arr == nullptr) {
          _abort("Null pointer exception", "??", 0, 0, 0, 0);
      }
      return arr->capacity();
  }
  template<typename T>
  void set_capacity(dynamic_array<T>* arr, int size) {
      if (arr == nullptr) {
          _abort("Null pointer exception", "??", 0, 0, 0, 0);
      }
      arr->reserve(size);
  }
  template<typename T>
  ObjectRef<dynamic_array<T>> clone(dynamic_array<T>* arr) {
      if (arr == nullptr) {
          _abort("Null pointer exception", "??", 0, 0, 0, 0);
      }
      dynamic_array<T>* ret = new dynamic_array<T>();
      ret->resize(arr->size());
      for(int i = 0; i < arr->size(); i++) {
          ret[i] = arr[i];
      }
      return ret;
  }
}

namespace _String_ {
  string substring(string str, int start, int end) {
      start = ((start % str.size()) + str.size()) % str.size();
      end = ((end % str.size()) + str.size()) % str.size();
      if (start < end) {
          return str.substr(start, end - start);
      } else {
          return string("", 0);
      }
  }
  ObjectRef<dynamic_array<string>> split(string str, string split_on) {
      dynamic_array<string>* ret = new dynamic_array<string>();
      int s;
      int split_size = split_on.size();
      while((s = str.find(split_on)) != -1) {
          ret->push_back(str.substr(0, s));
          str = string(str.data(), s + split_size);
      }
      ret->push_back(str);
      return ret;
  }
  int match(string str, string match_str) {
      return str.find(match_str);
  }
  int size(string str) {
      return str.size();
  }
  string concat(string self, string str) {
      char* buf = new char[self.size() + str.size()];
      self.copy(buf, self.size());
      str.copy(buf + self.size(), str.size());

      return string(buf, self.size() + str.size());
  }
  bool is_equal(string self, string str) {
      if (self.size() != str.size()) {
          return false;
      } else {
          return __builtin_memcmp(self.data(), str.data(), self.size()) == 0;
      }
  }
  int to_integer(string self) {
    unsigned int ret = 0;
    bool neg = false;
    for(int i = 0; i < self.size(); i++) {
        if (i == 0 && self.data()[i] == '-') neg = true;
        char c = self.data()[i];
        if (c < '0' || c > '9') {
            return false;
        }
        ret *= 10;
        ret += (c - '0');
    }
    return neg ? -(int)ret : (int)ret;
  }
  bool is_integer(string self) {
    unsigned int ret = 0;
    for(int i = 0; i < self.size(); i++) {
        if (i == 0 && self.data()[i] == '-') continue;
        char c = self.data()[i];
        if (c < '0' || c > '9') {
            return false;
        }
        ret *= 10;
        ret += (c - '0');
    }
    return true;
  }
}

/*
Export environment using these structs
*/

struct _Env_String {
    int size;
    void* str;
    _Env_String(int size, void* str) : size(size), str(str) {}
};

struct _Env_Array {
    int size;
    void* buf;
    _Env_Array(int size, void* buf) : size(size), buf(buf) {}
};

// **************
// Global Traits
// **************

namespace _Trait_Printable {
    TRAIT_HEADER
        static const id_type trait_id = 1;
    TRAIT_MID1
        typedef string (*_Function_to_string_type)(Object*);
        _Vtable(
            _Function_to_string_type _Function_to_string
        ) :
            _Function_to_string(_Function_to_string)
        {};
        _Function_to_string_type _Function_to_string;
    TRAIT_MID
        // Dynamic dispatch of trait function calls
        static string _Function_to_string(Object* object) {
            return ((_Vtable*)vtbls[object->object_id][trait_id])->_Function_to_string(object);
        }
    TRAIT_FOOTER
}

// **************
// Global functions
// **************

// Overload cout for arrays

string_buffer& operator<<(string_buffer& os, Object* v)
{
    if (is_trait<_Trait_Printable::_Instance>(v)) {
        os << ((_Trait_Printable::_Instance::_Vtable*)vtbls[v->object_id][_Trait_Printable::_Instance::trait_id])->_Function_to_string(v);
    } else {
        if (v) {
            long long a = (long long)v;
            os << "Object<id=" << v->object_id << ",instance=" << a << ">";
        } else {
            os << "Object<null>";
        }
    }
    return os; 
}

template <typename T> 
string_buffer& operator<<(string_buffer& os, ObjectRef<dynamic_array<T>>& v)
{ 
    os << "["; 
    for (int i = 0; i < v->size(); ++i) { 
        os << (*v)[i]; 
        if (i != v->size() - 1) 
            os << ", "; 
    } 
    os << "]";
    return os; 
}

// Variadic print statement

template<typename Value, typename... Values>
void _VS_print( Value v, Values... vs )
{
    string_buffer out(4096);

    using expander = int[];
    out << v; // first
    (void) expander{ 0, (out << " " << vs, void(), 0)... };
    out << "\n";
    out.flush();
#ifdef _COMPILE_VS_NATIVE_
    std::cout.write(out.data(), out.size());
#else
    g_print(out.to_string());
#endif
}
void _VS_print()
{
    string_buffer out(4096);
    out << "\n";
    out.flush();
#ifdef _COMPILE_VS_NATIVE_
    std::cout.write(out.data(), out.size());
#else
    g_print(out.to_string());
#endif
}

double _VS_sqrt(double in) {
    return __builtin_sqrt(in);
}

template<typename Value, typename... Values>
void _VS_raw_print( Value v, Values... vs )
{
    string_buffer out(4096);

    using expander = int[];
    out << v; // first
    (void) expander{ 0, (out << vs, void(), 0)... };
    out.flush();
#ifdef _COMPILE_VS_NATIVE_
    std::cout.write(out.data(), out.size());
#else
    g_print(out.to_string());
#endif
}
void _VS_raw_print()
{
}

string _VS_input()
{
#ifdef _COMPILE_VS_NATIVE_
    std::string* ret = new std::string();
    std::getline(std::cin, *ret);
    return string(ret->c_str(), ret->size());
#else
    return string("", 0);
#endif
}

void _VS_sleep(int ms) {
  if (ms > 0) {
#ifdef _COMPILE_VS_NATIVE_
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
  }
}

double _VS_time() {
#ifdef _COMPILE_VS_NATIVE_
  struct timespec spec;
  clock_gettime(CLOCK_REALTIME, &spec);

  return spec.tv_sec + spec.tv_nsec / 1e9;
#else
  return 0;
#endif
}

#ifdef _COMPILE_VS_NATIVE_
#include <random>
#include <limits>
#endif

int _VS_randi() {
#ifdef _COMPILE_VS_NATIVE_
    static std::random_device rd;
    static std::mt19937 gen(rd());
    // Random integer between 0 and 2^31 - 1
    return (int)(gen() >> 1);
#else
    return 4; // Random Enough
#endif
}

double _VS_randf() {
#ifdef _COMPILE_VS_NATIVE_
    static std::random_device rd;
    static std::mt19937 gen(rd());
    unsigned int a = gen();
    while(a == 0) {
        a = gen();
    }
    return a / ((double)(1ULL << 32));
#else
    return 0.5; // Random Enough
#endif
}

#ifdef _COMPILE_VS_NATIVE_
#define MAIN int main() {return 0;}
#else
#define MAIN
#endif

// **************
// End Global functions
// **************
