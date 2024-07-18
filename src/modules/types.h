#ifndef XI_TYPES_H_
# define XI_TYPES_H_

# include "raylib.h"
# include "stdint.h"
# include "stddef.h"

// Utilities
# define MAX(a, b) ((a)>(b)? (a) : (b))
# define MIN(a, b) ((a)<(b)? (a) : (b))

// Alias for raylib types
typedef Vector2 V2;
typedef Rectangle Rect;

// Types
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef i32 b32;
typedef char byte;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#endif // XI_TYPES_H_
