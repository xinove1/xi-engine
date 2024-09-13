#ifndef XI_CORE_H_
# define XI_CORE_H_

# include "raylib.h"
# include "stdint.h"
# include "stddef.h"

# define internal         static // Static func
# define internal_inline  static inline // Static inline func // TODO  better name
# define global           static // Static Variable global to tranlation unit
# define local            static // Static Variable local to func

#if defined(__clang__)
# define COMPILER_CLANG 1
#elif defined(_MSC_VER)
# define COMPILER_MSVC 1
#elif defined(__GNUC__) || defined(__GNUG__)
# define COMPILER_GCC 1
#else
# error Compiler not supported.
#endif

#if COMPILER_CLANG || COMPILER_GCC
# ifdef BUILD_DEBUG
#  define Trap() __builtin_unreachable()
# else
#  define Trap() __builtin_trap()
# endif
#elif COMPILER_MSVC
# define Trap() __debugbreak()
#else
# error Unknown trap intrinsic for this compiler.
#endif

//#define Trap() 
#define Expect(x) do {                                               \
    if (!(x)) {                                                      \
      fprintf(stderr, "Fatal error: %s:%d: assertion '%s' failed\n", \
        __FILE__, __LINE__, #x);                                     \
      Trap();                                                       \
    }                                                                \
  } while (0)

//#define AssertAlways(x) do{if(!(x)) {Trap();}}while(0)
#define AssertAlways(x) Expect(x)
#if BUILD_DEBUG
# define Assert(x) AssertAlways(x)
#else
# define Assert(x) (void)(x)
#endif

// Utilities
// 
#define count_of(a)    (size)(sizeof(a) / sizeof(*(a)))
#define length_of(s)   (countof(s) - 1)

# define MAX(a, b) ((a)>(b)? (a) : (b))
# define MIN(a, b) ((a)<(b)? (a) : (b))

# define KB(n)  (((u64)(n)) << 10)
# define MB(n)  (((u64)(n)) << 20)
# define GB(n)  (((u64)(n)) << 30)
# define TB(n)  (((u64)(n)) << 40)
# define Thousand(n)   ((n)*1000)
# define Million(n)    ((n)*1000000)
# define Billion(n)    ((n)*1000000000)

// Alias for raylib types 
typedef Vector2 V2;
typedef Vector3 V3;
typedef Rectangle Rect;

// Types
typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

typedef char cstr;
typedef i32  b32;
typedef char byte; // TODO  Update to unsigned char? or u8

typedef ptrdiff_t size;
typedef size_t    usize;

#endif // XI_CORE_H_
