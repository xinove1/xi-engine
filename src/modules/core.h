#ifndef XI_CORE_H_
# define XI_CORE_H_

# include "stdint.h"
# include "stddef.h"

// Metaprogram 
#define introspect(params)  

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
#define offset_of(s, m) ((size) &((s *) 0)->m)

# define MAX(a, b) ((a)>(b)? (a) : (b))
# define MIN(a, b) ((a)<(b)? (a) : (b))

# define KB(n)  (((u64)(n)) << 10)
# define MB(n)  (((u64)(n)) << 20)
# define GB(n)  (((u64)(n)) << 30)
# define TB(n)  (((u64)(n)) << 40)
# define Thousand(n)   ((n)*1000)
# define Million(n)    ((n)*1000000)
# define Billion(n)    ((n)*1000000000)

// ---------- Types ----------
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

// ---------- Define Raylib types if not defined ----------
// will cause collision if included before raylib

#ifdef NO_RAYLIB

# ifndef RL_BOOL_TYPE
   typedef enum bool { false = 0, true = !false } bool;
#  define RL_BOOL_TYPE
# endif

# ifndef RL_COLOR_TYPE
typedef struct Color {
    unsigned char r;        // Color red value
    unsigned char g;        // Color green value
    unsigned char b;        // Color blue value
    unsigned char a;        // Color alpha value
} Color;
# endif

# ifndef RL_RECTANGLE_TYPE
typedef struct Rectangle {
    f32 x;      
    f32 y;      
    f32 width;  
    f32 height; 
} Rectangle;
#  define RL_RECTANGLE_TYPE
# endif

# ifndef RL_VECTOR2_TYPE
typedef struct Vector2 {
    f32 x;
    f32 y;
} Vector2;
#  define RL_VECTOR2_TYPE
# endif

# ifndef RL_VECTOR3_TYPE
typedef struct Vector3 {
    f32 x;
    f32 y;
    f32 z;
} Vector3;
#  define RL_VECTOR3_TYPE
# endif

# ifndef RL_VECTOR4_TYPE
typedef struct Vector4 {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vector4;
#  define RL_VECTOR4_TYPE
# endif

# ifndef RL_QUATERNION_TYPE
typedef Vector4 Quaternion;
#  define RL_QUATERNION_TYPE
# endif

# ifndef RL_MATRIX_TYPE
typedef struct Matrix {
    f32 m0, m4, m8, m12;  // Matrix first row (4 components)
    f32 m1, m5, m9, m13;  // Matrix second row (4 components)
    f32 m2, m6, m10, m14; // Matrix third row (4 components)
    f32 m3, m7, m11, m15; // Matrix fourth row (4 components)
} Matrix;
#  define RL_MATRIX_TYPE
# endif
#endif

// ---------- Alias ----------
typedef struct Vector2 V2;
typedef struct Vector3 V3;
typedef struct Rectangle Rect;



// ---------- Simple helpers ----------
// NOTE  Move to string lib once created?

inline internal b32 IsEndOfLine(char c) {
	return ((c == '\n') || (c == '\r'));
}

inline internal b32 IsWhiteSpace(char c) {
	return ((c == ' ') || (c == '\n') || (c == '\t') || (c == '\r') || (c == '\v')|| (c == '\f'));
}

inline internal b32 IsAlpha(char c) {
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

inline internal b32 IsNumber(char c) {
	return (c >= '0' && c <= '9');
}

inline internal b32 IsAscii(char c) {
	return (c >= 0 && c <= 127);
}

inline internal b32 IsAlphaNumeric(char c) {
	return ((c >= 'a' && c <= 'z')
		|| (c >= 'A' && c <= 'Z')
		|| (c >= '0' && c <= '9'));
}

#endif // XI_CORE_H_
