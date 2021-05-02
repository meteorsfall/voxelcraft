
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

#ifndef _COMPILE_VS_NATIVE_
#endif

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
string_buffer& operator<<(string_buffer& os, int integer)
{
    char buf[16];
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
        buf[start+i] = buf[len-start-1-i];
        buf[len-start-1-i] = tmp;
    }
    os.write(buf, len);
    return os; 
}
string_buffer& operator<<(string_buffer& os, long long integer)
{
    char buf[16];
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
        buf[start+i] = buf[len-start-1-i];
        buf[len-start-1-i] = tmp;
    }
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
string_buffer& operator<<(string_buffer& os, double val)
{
    int first = (int)val;
    os << first;
    os << '.';
    int second = (val - first) * 100000;
    os << second;
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
    std::cerr << out.data();
    std::cerr.flush();
    exit(-1);
#else
    g_abort(out.to_string());
    while(true);
#endif
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
        if (index >= _size) {
            _abort("Array Index out-of-bounds", "??", 0, 0, 0, 0);
        }
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
        obj->reference_count++;
    }
    // If we can implicitly convert from U* to T*,
    // then we can create a ObjectRef<T> from an ObjectRef<U>
    template<typename U>
    ObjectRef(ObjectRef<U> other) {
        // Grab reference to Object
        obj = other.obj;
        // Nullify the other ObjectRef
        other.obj = nullptr;
    }
    ~ObjectRef()
    {
        if (obj) {
            obj->reference_count--;
            if (obj->reference_count == 0) {
                delete obj;
            }
        }
    }
    ObjectRef(const ObjectRef<T>& other) // copy constructor
    : ObjectRef(other.obj)
    {}
    ObjectRef(ObjectRef<T>&& other) noexcept // move constructor
    {
        this->obj = other.obj;
        other.obj = nullptr;
    }
    ObjectRef<T>& operator=(const ObjectRef<T>& other) // copy assignment
    {
        return *this = ObjectRef(other);
    }
    ObjectRef<T>& operator=(ObjectRef<T>&& other) noexcept // move assignment
    {
        standard::swap(obj, other.obj);
        return *this;
    }
    T& operator*() const
    {
        if (obj == nullptr) {
            _abort("Null pointer exception", "??", 0, 0, 0, 0);
        }
        return *obj;
    }
    T* operator->() const
    {
        if (obj == nullptr) {
            _abort("Null pointer exception", "??", 0, 0, 0, 0);
        }
        return obj;
    }
    // Define implicit conversion to T*
    operator T*() const { return obj; }
};

// Trait instance
class Trait {
public:
    Object* obj;
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
      typedef void (*_Function_print_type)(Object*);
      _Vtable(
          _Function_print_type _Function_print
      ) :
          _Function_print(_Function_print)
      {};
      _Function_print_type _Function_print;
  TRAIT_MID
      // Dynamic dispatch of trait function calls
      static void _Function_print(Object* object) {
          ((_Vtable*)vtbls[object->object_id][trait_id])->_Function_print(object);
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
        ((_Trait_Printable::_Instance::_Vtable*)vtbls[v->object_id][_Trait_Printable::_Instance::trait_id])->_Function_print(v);
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
    return _VS_randi() / ((double)(1u << 31));
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
