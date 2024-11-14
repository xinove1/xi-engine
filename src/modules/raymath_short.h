/**********************************************************************************************
*   Modified Version of raymath v1.5, shorting funcs names, changing types and removing unusead funcs
*      raymath v1.5 - Math functions to work with Vector2, V3, Matrix and Quaternions
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2015-2023 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/
#ifndef RAYMATH_SHORT_H_
# define RAYMATH_SHORT_H_
# include "core.h"
# include <math.h>

#ifdef RAYMATH_SHORT_IMPLEMENTATION
# define RMSAPI extern inline
#else
# define RMSAPI inline
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
#if !defined(RL_VECTOR2_TYPE)
// Vector2 type
typedef struct Vector2 {
    float x;
    float y;
} V2;
#define RL_VECTOR2_TYPE
#endif

#if !defined(RL_VECTOR3_TYPE)
// Vector3 type
typedef struct Vector3 {
    float x;
    float y;
    float z;
} V3;
#define RL_VECTOR3_TYPE
#endif

#if !defined(RL_VECTOR4_TYPE)
// Vector4 type
typedef struct Vector4 {
    float x;
    float y;
    float z;
    float w;
} V4;
#define RL_VECTOR4_TYPE
#endif

#if !defined(RL_QUATERNION_TYPE)
// Quaternion type
typedef Vector4 Quaternion;
#define RL_QUATERNION_TYPE
#endif

#if !defined(RL_MATRIX_TYPE)
// Matrix type (OpenGL style 4x4 - right handed, column major)
typedef struct Matrix {
    float m0, m4, m8, m12;      // Matrix first row (4 components)
    float m1, m5, m9, m13;      // Matrix second row (4 components)
    float m2, m6, m10, m14;     // Matrix third row (4 components)
    float m3, m7, m11, m15;     // Matrix fourth row (4 components)
} Matrix;
#define RL_MATRIX_TYPE
#endif

// NOTE: Helper types to be used instead of array return types for *ToFloat functions
typedef struct {
    f32 v[3];
} f32_3;

typedef struct {
    f32 v[16];
} f32_16;

//----------------------------------------------------------------------------------
// Creator helpers
//----------------------------------------------------------------------------------

#define Vec2(x, y) (V2) {x, y}
#define Vec2v(v) (V2) {v, v}
#define V2i32(vec) (V2) {(i32) vec.x, (i32) vec.y}
#define Rec(x, y, width, height) (Rect) {x, y, width, height}
#define RecV2(pos, size) (Rect) {.x = pos.x, .y = pos.y, .width = size.x, .height = size.y}
#define RecGrow(rec, amount) (Rect) {rec.x - amount, rec.y - amount, rec.width + amount * 2, rec.height + amount * 2}
#define RecPos(rec) (V2) {rec.x, rec.y}
#define RecSize(rec) (V2) {rec.width, rec.height}
#define Col(r, g, b) (Color) {r, g, b, 255}
#define ColA(r, g, b, a) (Color) {r, g, b, a}
#define ColV(v) (Color) {v, v, v, a}
#define ColVA(v, a) (Color) {v, v, v, a}

# define V2Zero() (V2){0, 0}
# define RectZero() (Rect) {0, 0, 0, 0}

RMSAPI b32 V2Compare(V2 v1, V2 v2) 
{
    return (v1.x == v2.x && v1.y == v2.y);
}

RMSAPI b32 RectCompare(Rect rec1, Rect rec2) 
{
    return (rec1.x == rec2.x && rec1.y == rec2.y && rec1.width == rec2.width && rec1.height == rec2.height);
}

RMSAPI V2 V2Absolute(V2 v) {
	return ((V2) {fabs(v.x), fabs(v.y)});
}

#define V2DirTo(from, to) (V2Normalize(V2Subtract(to, from)))

RMSAPI Color ColorAdd(Color c1, Color c2) 
{
    return ((Color) {c1.r + c2.r, c1.g + c2.g, c1.b + c2.b, c1.a + c2.a});
}

RMSAPI Color ColorSubtract(Color c1, Color c2) 
{
    return ((Color) {c1.r - c2.r, c1.g - c2.g, c1.b - c2.b, c1.a - c2.a});
}

RMSAPI Color ColorScale(Color color, f32 amount)
{
    return ((Color) {color.r * amount, color.g * amount, color.b * amount, color.a * amount});
}

RMSAPI b32 ColorCompare(Color c1, Color c2)
{
    return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a);
}

// Don't compare alpha
RMSAPI b32 ColorCompar2(Color c1, Color c2) 
{
    return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b);
}


//----------------------------------------------------------------------------------
// Module Functions Definition - Utils math
//----------------------------------------------------------------------------------
//
// Prepend f32 to names to not collide with raylibs use of raymath, (seems to only matter in static builds)

// Clamp f32 value
RMSAPI f32 f32Clamp(f32 value, f32 min, f32 max)
{
    f32 result = (value < min)? min : value;

    if (result > max) result = max;

    return result;
}

// Calculate linear interpolation between two f32s
RMSAPI f32 f32Lerp(f32 start, f32 end, f32 amount)
{
    f32 result = start + amount*(end - start);

    return result;
}

// Normalize input value within input range
RMSAPI f32 f32Normalize(f32 value, f32 start, f32 end)
{
    f32 result = (value - start)/(end - start);

    return result;
}

// Remap input value within input range to output range
RMSAPI f32 f32Remap(f32 value, f32 inputStart, f32 inputEnd, f32 outputStart, f32 outputEnd)
{
    f32 result = (value - inputStart)/(inputEnd - inputStart)*(outputEnd - outputStart) + outputStart;

    return result;
}

// Wrap input value from min to max
RMSAPI f32 f32Wrap(f32 value, f32 min, f32 max)
{
    f32 result = value - (max - min)*floorf((value - min)/(max - min));

    return result;
}

// Check whether two given f32s are almost equal
RMSAPI i32 f32Equals(f32 x, f32 y)
{
#if !defined(EPSILON)
    #define EPSILON 0.000001f
#endif

    i32 result = (fabsf(x - y)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y))));

    return result;
}

//----------------------------------------------------------------------------------
// Module Functions Definition - V2 math
//----------------------------------------------------------------------------------
// RMSAPI V2 V2Zero(void)
// {
//     V2 result = { 0.0f, 0.0f };

//     return result;
// }

// Vector with components value 1.0f
RMSAPI V2 V2One(void)
{
    V2 result = { 1.0f, 1.0f };

    return result;
}

// Add two vectors (v1 + v2)
RMSAPI V2 V2Add(V2 v1, V2 v2)
{
    V2 result = { v1.x + v2.x, v1.y + v2.y };

    return result;
}

// Add vector and f32 value
RMSAPI V2 V2AddValue(V2 v, f32 add)
{
    V2 result = { v.x + add, v.y + add };

    return result;
}

// Subtract two vectors (v1 - v2)
RMSAPI V2 V2Subtract(V2 v1, V2 v2)
{
    V2 result = { v1.x - v2.x, v1.y - v2.y };

    return result;
}

// Subtract vector by f32 value
RMSAPI V2 V2SubtractValue(V2 v, f32 sub)
{
    V2 result = { v.x - sub, v.y - sub };

    return result;
}

// Calculate vector length
RMSAPI f32 V2Length(V2 v)
{
    f32 result = sqrtf((v.x*v.x) + (v.y*v.y));

    return result;
}

// Calculate vector square length
RMSAPI f32 V2LengthSqr(V2 v)
{
    f32 result = (v.x*v.x) + (v.y*v.y);

    return result;
}

// Calculate two vectors dot product
RMSAPI f32 V2DotProduct(V2 v1, V2 v2)
{
    f32 result = (v1.x*v2.x + v1.y*v2.y);

    return result;
}

// Calculate distance between two vectors
RMSAPI f32 V2Distance(V2 v1, V2 v2)
{
    f32 result = sqrtf((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y));

    return result;
}

// Calculate square distance between two vectors
RMSAPI f32 V2DistanceSqr(V2 v1, V2 v2)
{
    f32 result = ((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y));

    return result;
}

// Calculate angle between two vectors
// NOTE: Angle is calculated from origin point (0, 0)
RMSAPI f32 V2Angle(V2 v1, V2 v2)
{
    f32 result = 0.0f;

    f32 dot = v1.x*v2.x + v1.y*v2.y;
    f32 det = v1.x*v2.y - v1.y*v2.x;

    result = atan2f(det, dot);

    return result;
}

// Calculate angle defined by a two vectors line
// NOTE: Parameters need to be normalized
// Current implementation should be aligned with glm::angle
RMSAPI f32 V2LineAngle(V2 start, V2 end)
{
    f32 result = 0.0f;

    // TODO(10/9/2023): Currently angles move clockwise, determine if this is wanted behavior
    result = -atan2f(end.y - start.y, end.x - start.x);

    return result;
}

// Scale vector (multiply by value)
RMSAPI V2 V2Scale(V2 v, f32 scale)
{
    V2 result = { v.x*scale, v.y*scale };

    return result;
}

// Multiply vector by vector
RMSAPI V2 V2Multiply(V2 v1, V2 v2)
{
    V2 result = { v1.x*v2.x, v1.y*v2.y };

    return result;
}

// Negate vector
RMSAPI V2 V2Negate(V2 v)
{
    V2 result = { -v.x, -v.y };

    return result;
}

// Divide vector by vector
RMSAPI V2 V2Divide(V2 v1, V2 v2)
{
    V2 result = { v1.x/v2.x, v1.y/v2.y };

    return result;
}

// Normalize provided vector
RMSAPI V2 V2Normalize(V2 v)
{
    V2 result = { 0 };
    f32 length = sqrtf((v.x*v.x) + (v.y*v.y));

    if (length > 0)
    {
        f32 ilength = 1.0f/length;
        result.x = v.x*ilength;
        result.y = v.y*ilength;
    }

    return result;
}

// Transforms a V2 by a given Matrix
RMSAPI V2 V2Transform(V2 v, Matrix mat)
{
    V2 result = { 0 };

    f32 x = v.x;
    f32 y = v.y;
    f32 z = 0;

    result.x = mat.m0*x + mat.m4*y + mat.m8*z + mat.m12;
    result.y = mat.m1*x + mat.m5*y + mat.m9*z + mat.m13;

    return result;
}

// Calculate linear interpolation between two vectors
RMSAPI V2 V2Lerp(V2 v1, V2 v2, f32 amount)
{
    V2 result = { 0 };

    result.x = v1.x + amount*(v2.x - v1.x);
    result.y = v1.y + amount*(v2.y - v1.y);

    return result;
}

// Calculate reflected vector to normal
RMSAPI V2 V2Reflect(V2 v, V2 normal)
{
    V2 result = { 0 };

    f32 dotProduct = (v.x*normal.x + v.y*normal.y); // Dot product

    result.x = v.x - (2.0f*normal.x)*dotProduct;
    result.y = v.y - (2.0f*normal.y)*dotProduct;

    return result;
}

// Rotate vector by angle
RMSAPI V2 V2Rotate(V2 v, f32 angle)
{
    V2 result = { 0 };

    f32 cosres = cosf(angle);
    f32 sinres = sinf(angle);

    result.x = v.x*cosres - v.y*sinres;
    result.y = v.x*sinres + v.y*cosres;

    return result;
}

// Move Vector towards target
RMSAPI V2 V2MoveTowards(V2 v, V2 target, f32 maxDistance)
{
    V2 result = { 0 };

    f32 dx = target.x - v.x;
    f32 dy = target.y - v.y;
    f32 value = (dx*dx) + (dy*dy);

    if ((value == 0) || ((maxDistance >= 0) && (value <= maxDistance*maxDistance))) return target;

    f32 dist = sqrtf(value);

    result.x = v.x + dx/dist*maxDistance;
    result.y = v.y + dy/dist*maxDistance;

    return result;
}

// Invert the given vector
RMSAPI V2 V2Invert(V2 v)
{
    V2 result = { 1.0f/v.x, 1.0f/v.y };

    return result;
}

// Clamp the components of the vector between
// min and max values specified by the given vectors
RMSAPI V2 V2Clamp(V2 v, V2 min, V2 max)
{
    V2 result = { 0 };

    result.x = fminf(max.x, fmaxf(min.x, v.x));
    result.y = fminf(max.y, fmaxf(min.y, v.y));

    return result;
}

// Clamp the magnitude of the vector between two min and max values
RMSAPI V2 V2ClampValue(V2 v, f32 min, f32 max)
{
    V2 result = v;

    f32 length = (v.x*v.x) + (v.y*v.y);
    if (length > 0.0f)
    {
        length = sqrtf(length);

        if (length < min)
        {
            f32 scale = min/length;
            result.x = v.x*scale;
            result.y = v.y*scale;
        }
        else if (length > max)
        {
            f32 scale = max/length;
            result.x = v.x*scale;
            result.y = v.y*scale;
        }
    }

    return result;
}

// Check whether two given vectors are almost equal
RMSAPI i32 V2Equals(V2 p, V2 q)
{
#if !defined(EPSILON)
    #define EPSILON 0.000001f
#endif

    i32 result = ((fabsf(p.x - q.x)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.x), fabsf(q.x))))) &&
                  ((fabsf(p.y - q.y)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.y), fabsf(q.y)))));

    return result;
}

//----------------------------------------------------------------------------------
// Module Functions Definition - V3 math
//----------------------------------------------------------------------------------

// Vector with components value 0.0f
RMSAPI V3 V3Zero(void)
{
    V3 result = { 0.0f, 0.0f, 0.0f };

    return result;
}

// Vector with components value 1.0f
RMSAPI V3 V3One(void)
{
    V3 result = { 1.0f, 1.0f, 1.0f };

    return result;
}

// Add two vectors
RMSAPI V3 V3Add(V3 v1, V3 v2)
{
    V3 result = { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };

    return result;
}

// Add vector and f32 value
RMSAPI V3 V3AddValue(V3 v, f32 add)
{
    V3 result = { v.x + add, v.y + add, v.z + add };

    return result;
}

// Subtract two vectors
RMSAPI V3 V3Subtract(V3 v1, V3 v2)
{
    V3 result = { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };

    return result;
}

// Subtract vector by f32 value
RMSAPI V3 V3SubtractValue(V3 v, f32 sub)
{
    V3 result = { v.x - sub, v.y - sub, v.z - sub };

    return result;
}

// Multiply vector by scalar
RMSAPI V3 V3Scale(V3 v, f32 scalar)
{
    V3 result = { v.x*scalar, v.y*scalar, v.z*scalar };

    return result;
}

// Multiply vector by vector
RMSAPI V3 V3Multiply(V3 v1, V3 v2)
{
    V3 result = { v1.x*v2.x, v1.y*v2.y, v1.z*v2.z };

    return result;
}

// Calculate two vectors cross product
RMSAPI V3 V3CrossProduct(V3 v1, V3 v2)
{
    V3 result = { v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x };

    return result;
}

// Calculate one vector perpendicular vector
RMSAPI V3 V3Perpendicular(V3 v)
{
    V3 result = { 0 };

    f32 min = (f32) fabs(v.x);
    V3 cardinalAxis = {1.0f, 0.0f, 0.0f};

    if (fabsf(v.y) < min)
    {
        min = (f32) fabs(v.y);
        V3 tmp = {0.0f, 1.0f, 0.0f};
        cardinalAxis = tmp;
    }

    if (fabsf(v.z) < min)
    {
        V3 tmp = {0.0f, 0.0f, 1.0f};
        cardinalAxis = tmp;
    }

    // Cross product between vectors
    result.x = v.y*cardinalAxis.z - v.z*cardinalAxis.y;
    result.y = v.z*cardinalAxis.x - v.x*cardinalAxis.z;
    result.z = v.x*cardinalAxis.y - v.y*cardinalAxis.x;

    return result;
}

// Calculate vector length
RMSAPI f32 V3Length(const V3 v)
{
    f32 result = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);

    return result;
}

// Calculate vector square length
RMSAPI f32 V3LengthSqr(const V3 v)
{
    f32 result = v.x*v.x + v.y*v.y + v.z*v.z;

    return result;
}

// Calculate two vectors dot product
RMSAPI f32 V3DotProduct(V3 v1, V3 v2)
{
    f32 result = (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);

    return result;
}

// Calculate distance between two vectors
RMSAPI f32 V3Distance(V3 v1, V3 v2)
{
    f32 result = 0.0f;

    f32 dx = v2.x - v1.x;
    f32 dy = v2.y - v1.y;
    f32 dz = v2.z - v1.z;
    result = sqrtf(dx*dx + dy*dy + dz*dz);

    return result;
}

// Calculate square distance between two vectors
RMSAPI f32 V3DistanceSqr(V3 v1, V3 v2)
{
    f32 result = 0.0f;

    f32 dx = v2.x - v1.x;
    f32 dy = v2.y - v1.y;
    f32 dz = v2.z - v1.z;
    result = dx*dx + dy*dy + dz*dz;

    return result;
}

// Calculate angle between two vectors
RMSAPI f32 V3Angle(V3 v1, V3 v2)
{
    f32 result = 0.0f;

    V3 cross = { v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x };
    f32 len = sqrtf(cross.x*cross.x + cross.y*cross.y + cross.z*cross.z);
    f32 dot = (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
    result = atan2f(len, dot);

    return result;
}

// Negate provided vector (invert direction)
RMSAPI V3 V3Negate(V3 v)
{
    V3 result = { -v.x, -v.y, -v.z };

    return result;
}

// Divide vector by vector
RMSAPI V3 V3Divide(V3 v1, V3 v2)
{
    V3 result = { v1.x/v2.x, v1.y/v2.y, v1.z/v2.z };

    return result;
}

// Normalize provided vector
RMSAPI V3 V3Normalize(V3 v)
{
    V3 result = v;

    f32 length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length != 0.0f)
    {
        f32 ilength = 1.0f/length;

        result.x *= ilength;
        result.y *= ilength;
        result.z *= ilength;
    }

    return result;
}

//Calculate the projection of the vector v1 on to v2
RMSAPI V3 V3Project(V3 v1, V3 v2)
{
    V3 result = { 0 };
    
    f32 v1dv2 = (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
    f32 v2dv2 = (v2.x*v2.x + v2.y*v2.y + v2.z*v2.z);

    f32 mag = v1dv2/v2dv2;

    result.x = v2.x*mag;
    result.y = v2.y*mag;
    result.z = v2.z*mag;

    return result;
}

//Calculate the rejection of the vector v1 on to v2
RMSAPI V3 V3Reject(V3 v1, V3 v2)
{
    V3 result = { 0 };
    
    f32 v1dv2 = (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
    f32 v2dv2 = (v2.x*v2.x + v2.y*v2.y + v2.z*v2.z);

    f32 mag = v1dv2/v2dv2;

    result.x = v1.x - (v2.x*mag);
    result.y = v1.y - (v2.y*mag);
    result.z = v1.z - (v2.z*mag);

    return result;
}

// Orthonormalize provided vectors
// Makes vectors normalized and orthogonal to each other
// Gram-Schmidt function implementation
RMSAPI void V3OrthoNormalize(V3 *v1, V3 *v2)
{
    f32 length = 0.0f;
    f32 ilength = 0.0f;

    // V3Normalize(*v1);
    V3 v = *v1;
    length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f/length;
    v1->x *= ilength;
    v1->y *= ilength;
    v1->z *= ilength;

    // V3CrossProduct(*v1, *v2)
    V3 vn1 = { v1->y*v2->z - v1->z*v2->y, v1->z*v2->x - v1->x*v2->z, v1->x*v2->y - v1->y*v2->x };

    // V3Normalize(vn1);
    v = vn1;
    length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f/length;
    vn1.x *= ilength;
    vn1.y *= ilength;
    vn1.z *= ilength;

    // V3CrossProduct(vn1, *v1)
    V3 vn2 = { vn1.y*v1->z - vn1.z*v1->y, vn1.z*v1->x - vn1.x*v1->z, vn1.x*v1->y - vn1.y*v1->x };

    *v2 = vn2;
}

// Transforms a V3 by a given Matrix
RMSAPI V3 V3Transform(V3 v, Matrix mat)
{
    V3 result = { 0 };

    f32 x = v.x;
    f32 y = v.y;
    f32 z = v.z;

    result.x = mat.m0*x + mat.m4*y + mat.m8*z + mat.m12;
    result.y = mat.m1*x + mat.m5*y + mat.m9*z + mat.m13;
    result.z = mat.m2*x + mat.m6*y + mat.m10*z + mat.m14;

    return result;
}

// Transform a vector by quaternion rotation
RMSAPI V3 V3RotateByQuaternion(V3 v, Quaternion q)
{
    V3 result = { 0 };

    result.x = v.x*(q.x*q.x + q.w*q.w - q.y*q.y - q.z*q.z) + v.y*(2*q.x*q.y - 2*q.w*q.z) + v.z*(2*q.x*q.z + 2*q.w*q.y);
    result.y = v.x*(2*q.w*q.z + 2*q.x*q.y) + v.y*(q.w*q.w - q.x*q.x + q.y*q.y - q.z*q.z) + v.z*(-2*q.w*q.x + 2*q.y*q.z);
    result.z = v.x*(-2*q.w*q.y + 2*q.x*q.z) + v.y*(2*q.w*q.x + 2*q.y*q.z)+ v.z*(q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z);

    return result;
}

// Rotates a vector around an axis
RMSAPI V3 V3RotateByAxisAngle(V3 v, V3 axis, f32 angle)
{
    // Using Euler-Rodrigues Formula
    // Ref.: https://en.wikipedia.org/w/index.php?title=Euler%E2%80%93Rodrigues_formula

    V3 result = v;

    // V3Normalize(axis);
    f32 length = sqrtf(axis.x*axis.x + axis.y*axis.y + axis.z*axis.z);
    if (length == 0.0f) length = 1.0f;
    f32 ilength = 1.0f / length;
    axis.x *= ilength;
    axis.y *= ilength;
    axis.z *= ilength;

    angle /= 2.0f;
    f32 a = sinf(angle);
    f32 b = axis.x*a;
    f32 c = axis.y*a;
    f32 d = axis.z*a;
    a = cosf(angle);
    V3 w = { b, c, d };

    // V3CrossProduct(w, v)
    V3 wv = { w.y*v.z - w.z*v.y, w.z*v.x - w.x*v.z, w.x*v.y - w.y*v.x };

    // V3CrossProduct(w, wv)
    V3 wwv = { w.y*wv.z - w.z*wv.y, w.z*wv.x - w.x*wv.z, w.x*wv.y - w.y*wv.x };

    // V3Scale(wv, 2*a)
    a *= 2;
    wv.x *= a;
    wv.y *= a;
    wv.z *= a;

    // V3Scale(wwv, 2)
    wwv.x *= 2;
    wwv.y *= 2;
    wwv.z *= 2;

    result.x += wv.x;
    result.y += wv.y;
    result.z += wv.z;

    result.x += wwv.x;
    result.y += wwv.y;
    result.z += wwv.z;

    return result;
}

// Calculate linear interpolation between two vectors
RMSAPI V3 V3Lerp(V3 v1, V3 v2, f32 amount)
{
    V3 result = { 0 };

    result.x = v1.x + amount*(v2.x - v1.x);
    result.y = v1.y + amount*(v2.y - v1.y);
    result.z = v1.z + amount*(v2.z - v1.z);

    return result;
}

// Calculate reflected vector to normal
RMSAPI V3 V3Reflect(V3 v, V3 normal)
{
    V3 result = { 0 };

    // I is the original vector
    // N is the normal of the incident plane
    // R = I - (2*N*(DotProduct[I, N]))

    f32 dotProduct = (v.x*normal.x + v.y*normal.y + v.z*normal.z);

    result.x = v.x - (2.0f*normal.x)*dotProduct;
    result.y = v.y - (2.0f*normal.y)*dotProduct;
    result.z = v.z - (2.0f*normal.z)*dotProduct;

    return result;
}

// Get min value for each pair of components
RMSAPI V3 V3Min(V3 v1, V3 v2)
{
    V3 result = { 0 };

    result.x = fminf(v1.x, v2.x);
    result.y = fminf(v1.y, v2.y);
    result.z = fminf(v1.z, v2.z);

    return result;
}

// Get max value for each pair of components
RMSAPI V3 V3Max(V3 v1, V3 v2)
{
    V3 result = { 0 };

    result.x = fmaxf(v1.x, v2.x);
    result.y = fmaxf(v1.y, v2.y);
    result.z = fmaxf(v1.z, v2.z);

    return result;
}

// Compute barycenter coordinates (u, v, w) for point p with respect to triangle (a, b, c)
// NOTE: Assumes P is on the plane of the triangle
RMSAPI V3 V3Barycenter(V3 p, V3 a, V3 b, V3 c)
{
    V3 result = { 0 };

    V3 v0 = { b.x - a.x, b.y - a.y, b.z - a.z };   // V3Subtract(b, a)
    V3 v1 = { c.x - a.x, c.y - a.y, c.z - a.z };   // V3Subtract(c, a)
    V3 v2 = { p.x - a.x, p.y - a.y, p.z - a.z };   // V3Subtract(p, a)
    f32 d00 = (v0.x*v0.x + v0.y*v0.y + v0.z*v0.z);    // V3DotProduct(v0, v0)
    f32 d01 = (v0.x*v1.x + v0.y*v1.y + v0.z*v1.z);    // V3DotProduct(v0, v1)
    f32 d11 = (v1.x*v1.x + v1.y*v1.y + v1.z*v1.z);    // V3DotProduct(v1, v1)
    f32 d20 = (v2.x*v0.x + v2.y*v0.y + v2.z*v0.z);    // V3DotProduct(v2, v0)
    f32 d21 = (v2.x*v1.x + v2.y*v1.y + v2.z*v1.z);    // V3DotProduct(v2, v1)

    f32 denom = d00*d11 - d01*d01;

    result.y = (d11*d20 - d01*d21)/denom;
    result.z = (d00*d21 - d01*d20)/denom;
    result.x = 1.0f - (result.z + result.y);

    return result;
}

// Projects a V3 from screen space into object space
// NOTE: We are avoiding calling other raymath functions despite available
RMSAPI V3 V3Unproject(V3 source, Matrix projection, Matrix view)
{
    V3 result = { 0 };

    // Calculate unprojected matrix (multiply view matrix by projection matrix) and invert it
    Matrix matViewProj = {      // MatrixMultiply(view, projection);
        view.m0*projection.m0 + view.m1*projection.m4 + view.m2*projection.m8 + view.m3*projection.m12,
        view.m0*projection.m1 + view.m1*projection.m5 + view.m2*projection.m9 + view.m3*projection.m13,
        view.m0*projection.m2 + view.m1*projection.m6 + view.m2*projection.m10 + view.m3*projection.m14,
        view.m0*projection.m3 + view.m1*projection.m7 + view.m2*projection.m11 + view.m3*projection.m15,
        view.m4*projection.m0 + view.m5*projection.m4 + view.m6*projection.m8 + view.m7*projection.m12,
        view.m4*projection.m1 + view.m5*projection.m5 + view.m6*projection.m9 + view.m7*projection.m13,
        view.m4*projection.m2 + view.m5*projection.m6 + view.m6*projection.m10 + view.m7*projection.m14,
        view.m4*projection.m3 + view.m5*projection.m7 + view.m6*projection.m11 + view.m7*projection.m15,
        view.m8*projection.m0 + view.m9*projection.m4 + view.m10*projection.m8 + view.m11*projection.m12,
        view.m8*projection.m1 + view.m9*projection.m5 + view.m10*projection.m9 + view.m11*projection.m13,
        view.m8*projection.m2 + view.m9*projection.m6 + view.m10*projection.m10 + view.m11*projection.m14,
        view.m8*projection.m3 + view.m9*projection.m7 + view.m10*projection.m11 + view.m11*projection.m15,
        view.m12*projection.m0 + view.m13*projection.m4 + view.m14*projection.m8 + view.m15*projection.m12,
        view.m12*projection.m1 + view.m13*projection.m5 + view.m14*projection.m9 + view.m15*projection.m13,
        view.m12*projection.m2 + view.m13*projection.m6 + view.m14*projection.m10 + view.m15*projection.m14,
        view.m12*projection.m3 + view.m13*projection.m7 + view.m14*projection.m11 + view.m15*projection.m15 };

    // Calculate inverted matrix -> MatrixInvert(matViewProj);
    // Cache the matrix values (speed optimization)
    f32 a00 = matViewProj.m0, a01 = matViewProj.m1, a02 = matViewProj.m2, a03 = matViewProj.m3;
    f32 a10 = matViewProj.m4, a11 = matViewProj.m5, a12 = matViewProj.m6, a13 = matViewProj.m7;
    f32 a20 = matViewProj.m8, a21 = matViewProj.m9, a22 = matViewProj.m10, a23 = matViewProj.m11;
    f32 a30 = matViewProj.m12, a31 = matViewProj.m13, a32 = matViewProj.m14, a33 = matViewProj.m15;

    f32 b00 = a00*a11 - a01*a10;
    f32 b01 = a00*a12 - a02*a10;
    f32 b02 = a00*a13 - a03*a10;
    f32 b03 = a01*a12 - a02*a11;
    f32 b04 = a01*a13 - a03*a11;
    f32 b05 = a02*a13 - a03*a12;
    f32 b06 = a20*a31 - a21*a30;
    f32 b07 = a20*a32 - a22*a30;
    f32 b08 = a20*a33 - a23*a30;
    f32 b09 = a21*a32 - a22*a31;
    f32 b10 = a21*a33 - a23*a31;
    f32 b11 = a22*a33 - a23*a32;

    // Calculate the invert determinant (inlined to avoid double-caching)
    f32 invDet = 1.0f/(b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06);

    Matrix matViewProjInv = {
        (a11*b11 - a12*b10 + a13*b09)*invDet,
        (-a01*b11 + a02*b10 - a03*b09)*invDet,
        (a31*b05 - a32*b04 + a33*b03)*invDet,
        (-a21*b05 + a22*b04 - a23*b03)*invDet,
        (-a10*b11 + a12*b08 - a13*b07)*invDet,
        (a00*b11 - a02*b08 + a03*b07)*invDet,
        (-a30*b05 + a32*b02 - a33*b01)*invDet,
        (a20*b05 - a22*b02 + a23*b01)*invDet,
        (a10*b10 - a11*b08 + a13*b06)*invDet,
        (-a00*b10 + a01*b08 - a03*b06)*invDet,
        (a30*b04 - a31*b02 + a33*b00)*invDet,
        (-a20*b04 + a21*b02 - a23*b00)*invDet,
        (-a10*b09 + a11*b07 - a12*b06)*invDet,
        (a00*b09 - a01*b07 + a02*b06)*invDet,
        (-a30*b03 + a31*b01 - a32*b00)*invDet,
        (a20*b03 - a21*b01 + a22*b00)*invDet };

    // Create quaternion from source point
    Quaternion quat = { source.x, source.y, source.z, 1.0f };

    // Multiply quat point by unprojecte matrix
    Quaternion qtransformed = {     // QuaternionTransform(quat, matViewProjInv)
        matViewProjInv.m0*quat.x + matViewProjInv.m4*quat.y + matViewProjInv.m8*quat.z + matViewProjInv.m12*quat.w,
        matViewProjInv.m1*quat.x + matViewProjInv.m5*quat.y + matViewProjInv.m9*quat.z + matViewProjInv.m13*quat.w,
        matViewProjInv.m2*quat.x + matViewProjInv.m6*quat.y + matViewProjInv.m10*quat.z + matViewProjInv.m14*quat.w,
        matViewProjInv.m3*quat.x + matViewProjInv.m7*quat.y + matViewProjInv.m11*quat.z + matViewProjInv.m15*quat.w };

    // Normalized world points in vectors
    result.x = qtransformed.x/qtransformed.w;
    result.y = qtransformed.y/qtransformed.w;
    result.z = qtransformed.z/qtransformed.w;

    return result;
}

// Get V3 as f32 array
RMSAPI f32_3 V3Tof32V(V3 v)
{
    f32_3 buffer = { 0 };

    buffer.v[0] = v.x;
    buffer.v[1] = v.y;
    buffer.v[2] = v.z;

    return buffer;
}

// Invert the given vector
RMSAPI V3 V3Invert(V3 v)
{
    V3 result = { 1.0f/v.x, 1.0f/v.y, 1.0f/v.z };

    return result;
}

// Clamp the components of the vector between
// min and max values specified by the given vectors
RMSAPI V3 V3Clamp(V3 v, V3 min, V3 max)
{
    V3 result = { 0 };

    result.x = fminf(max.x, fmaxf(min.x, v.x));
    result.y = fminf(max.y, fmaxf(min.y, v.y));
    result.z = fminf(max.z, fmaxf(min.z, v.z));

    return result;
}

// Clamp the magnitude of the vector between two values
RMSAPI V3 V3ClampValue(V3 v, f32 min, f32 max)
{
    V3 result = v;

    f32 length = (v.x*v.x) + (v.y*v.y) + (v.z*v.z);
    if (length > 0.0f)
    {
        length = sqrtf(length);

        if (length < min)
        {
            f32 scale = min/length;
            result.x = v.x*scale;
            result.y = v.y*scale;
            result.z = v.z*scale;
        }
        else if (length > max)
        {
            f32 scale = max/length;
            result.x = v.x*scale;
            result.y = v.y*scale;
            result.z = v.z*scale;
        }
    }

    return result;
}

// Check whether two given vectors are almost equal
RMSAPI int V3Equals(V3 p, V3 q)
{
#if !defined(EPSILON)
    #define EPSILON 0.000001f
#endif

    int result = ((fabsf(p.x - q.x)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.x), fabsf(q.x))))) &&
                 ((fabsf(p.y - q.y)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.y), fabsf(q.y))))) &&
                 ((fabsf(p.z - q.z)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.z), fabsf(q.z)))));

    return result;
}

// Compute the direction of a refracted ray
// v: normalized direction of the incoming ray
// n: normalized normal vector of the interface of two optical media
// r: ratio of the refractive index of the medium from where the ray comes
//    to the refractive index of the medium on the other side of the surface
RMSAPI V3 V3Refract(V3 v, V3 n, f32 r)
{
    V3 result = { 0 };

    f32 dot = v.x*n.x + v.y*n.y + v.z*n.z;
    f32 d = 1.0f - r*r*(1.0f - dot*dot);

    if (d >= 0.0f)
    {
        d = sqrtf(d);
        v.x = r*v.x - (r*dot + d)*n.x;
        v.y = r*v.y - (r*dot + d)*n.y;
        v.z = r*v.z - (r*dot + d)*n.z;

        result = v;
    }

    return result;
}

#ifdef RAYMATH_SHORT_MATRIX_QUATERNION // TODO 

//----------------------------------------------------------------------------------
// Module Functions Definition - Matrix math
//----------------------------------------------------------------------------------

// Compute matrix determinant
RMSAPI f32 MatrixDeterminant(Matrix mat)
{
    f32 result = 0.0f;

    // Cache the matrix values (speed optimization)
    f32 a00 = mat.m0, a01 = mat.m1, a02 = mat.m2, a03 = mat.m3;
    f32 a10 = mat.m4, a11 = mat.m5, a12 = mat.m6, a13 = mat.m7;
    f32 a20 = mat.m8, a21 = mat.m9, a22 = mat.m10, a23 = mat.m11;
    f32 a30 = mat.m12, a31 = mat.m13, a32 = mat.m14, a33 = mat.m15;

    result = a30*a21*a12*a03 - a20*a31*a12*a03 - a30*a11*a22*a03 + a10*a31*a22*a03 +
             a20*a11*a32*a03 - a10*a21*a32*a03 - a30*a21*a02*a13 + a20*a31*a02*a13 +
             a30*a01*a22*a13 - a00*a31*a22*a13 - a20*a01*a32*a13 + a00*a21*a32*a13 +
             a30*a11*a02*a23 - a10*a31*a02*a23 - a30*a01*a12*a23 + a00*a31*a12*a23 +
             a10*a01*a32*a23 - a00*a11*a32*a23 - a20*a11*a02*a33 + a10*a21*a02*a33 +
             a20*a01*a12*a33 - a00*a21*a12*a33 - a10*a01*a22*a33 + a00*a11*a22*a33;

    return result;
}

// Get the trace of the matrix (sum of the values along the diagonal)
RMSAPI f32 MatrixTrace(Matrix mat)
{
    f32 result = (mat.m0 + mat.m5 + mat.m10 + mat.m15);

    return result;
}

// Transposes provided matrix
RMSAPI Matrix MatrixTranspose(Matrix mat)
{
    Matrix result = { 0 };

    result.m0 = mat.m0;
    result.m1 = mat.m4;
    result.m2 = mat.m8;
    result.m3 = mat.m12;
    result.m4 = mat.m1;
    result.m5 = mat.m5;
    result.m6 = mat.m9;
    result.m7 = mat.m13;
    result.m8 = mat.m2;
    result.m9 = mat.m6;
    result.m10 = mat.m10;
    result.m11 = mat.m14;
    result.m12 = mat.m3;
    result.m13 = mat.m7;
    result.m14 = mat.m11;
    result.m15 = mat.m15;

    return result;
}

// Invert provided matrix
RMSAPI Matrix MatrixInvert(Matrix mat)
{
    Matrix result = { 0 };

    // Cache the matrix values (speed optimization)
    f32 a00 = mat.m0, a01 = mat.m1, a02 = mat.m2, a03 = mat.m3;
    f32 a10 = mat.m4, a11 = mat.m5, a12 = mat.m6, a13 = mat.m7;
    f32 a20 = mat.m8, a21 = mat.m9, a22 = mat.m10, a23 = mat.m11;
    f32 a30 = mat.m12, a31 = mat.m13, a32 = mat.m14, a33 = mat.m15;

    f32 b00 = a00*a11 - a01*a10;
    f32 b01 = a00*a12 - a02*a10;
    f32 b02 = a00*a13 - a03*a10;
    f32 b03 = a01*a12 - a02*a11;
    f32 b04 = a01*a13 - a03*a11;
    f32 b05 = a02*a13 - a03*a12;
    f32 b06 = a20*a31 - a21*a30;
    f32 b07 = a20*a32 - a22*a30;
    f32 b08 = a20*a33 - a23*a30;
    f32 b09 = a21*a32 - a22*a31;
    f32 b10 = a21*a33 - a23*a31;
    f32 b11 = a22*a33 - a23*a32;

    // Calculate the invert determinant (inlined to avoid double-caching)
    f32 invDet = 1.0f/(b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06);

    result.m0 = (a11*b11 - a12*b10 + a13*b09)*invDet;
    result.m1 = (-a01*b11 + a02*b10 - a03*b09)*invDet;
    result.m2 = (a31*b05 - a32*b04 + a33*b03)*invDet;
    result.m3 = (-a21*b05 + a22*b04 - a23*b03)*invDet;
    result.m4 = (-a10*b11 + a12*b08 - a13*b07)*invDet;
    result.m5 = (a00*b11 - a02*b08 + a03*b07)*invDet;
    result.m6 = (-a30*b05 + a32*b02 - a33*b01)*invDet;
    result.m7 = (a20*b05 - a22*b02 + a23*b01)*invDet;
    result.m8 = (a10*b10 - a11*b08 + a13*b06)*invDet;
    result.m9 = (-a00*b10 + a01*b08 - a03*b06)*invDet;
    result.m10 = (a30*b04 - a31*b02 + a33*b00)*invDet;
    result.m11 = (-a20*b04 + a21*b02 - a23*b00)*invDet;
    result.m12 = (-a10*b09 + a11*b07 - a12*b06)*invDet;
    result.m13 = (a00*b09 - a01*b07 + a02*b06)*invDet;
    result.m14 = (-a30*b03 + a31*b01 - a32*b00)*invDet;
    result.m15 = (a20*b03 - a21*b01 + a22*b00)*invDet;

    return result;
}

// Get identity matrix
RMSAPI Matrix MatrixIdentity(void)
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f };

    return result;
}

// Add two matrices
RMSAPI Matrix MatrixAdd(Matrix left, Matrix right)
{
    Matrix result = { 0 };

    result.m0 = left.m0 + right.m0;
    result.m1 = left.m1 + right.m1;
    result.m2 = left.m2 + right.m2;
    result.m3 = left.m3 + right.m3;
    result.m4 = left.m4 + right.m4;
    result.m5 = left.m5 + right.m5;
    result.m6 = left.m6 + right.m6;
    result.m7 = left.m7 + right.m7;
    result.m8 = left.m8 + right.m8;
    result.m9 = left.m9 + right.m9;
    result.m10 = left.m10 + right.m10;
    result.m11 = left.m11 + right.m11;
    result.m12 = left.m12 + right.m12;
    result.m13 = left.m13 + right.m13;
    result.m14 = left.m14 + right.m14;
    result.m15 = left.m15 + right.m15;

    return result;
}

// Subtract two matrices (left - right)
RMSAPI Matrix MatrixSubtract(Matrix left, Matrix right)
{
    Matrix result = { 0 };

    result.m0 = left.m0 - right.m0;
    result.m1 = left.m1 - right.m1;
    result.m2 = left.m2 - right.m2;
    result.m3 = left.m3 - right.m3;
    result.m4 = left.m4 - right.m4;
    result.m5 = left.m5 - right.m5;
    result.m6 = left.m6 - right.m6;
    result.m7 = left.m7 - right.m7;
    result.m8 = left.m8 - right.m8;
    result.m9 = left.m9 - right.m9;
    result.m10 = left.m10 - right.m10;
    result.m11 = left.m11 - right.m11;
    result.m12 = left.m12 - right.m12;
    result.m13 = left.m13 - right.m13;
    result.m14 = left.m14 - right.m14;
    result.m15 = left.m15 - right.m15;

    return result;
}

// Get two matrix multiplication
// NOTE: When multiplying matrices... the order matters!
RMSAPI Matrix MatrixMultiply(Matrix left, Matrix right)
{
    Matrix result = { 0 };

    result.m0 = left.m0*right.m0 + left.m1*right.m4 + left.m2*right.m8 + left.m3*right.m12;
    result.m1 = left.m0*right.m1 + left.m1*right.m5 + left.m2*right.m9 + left.m3*right.m13;
    result.m2 = left.m0*right.m2 + left.m1*right.m6 + left.m2*right.m10 + left.m3*right.m14;
    result.m3 = left.m0*right.m3 + left.m1*right.m7 + left.m2*right.m11 + left.m3*right.m15;
    result.m4 = left.m4*right.m0 + left.m5*right.m4 + left.m6*right.m8 + left.m7*right.m12;
    result.m5 = left.m4*right.m1 + left.m5*right.m5 + left.m6*right.m9 + left.m7*right.m13;
    result.m6 = left.m4*right.m2 + left.m5*right.m6 + left.m6*right.m10 + left.m7*right.m14;
    result.m7 = left.m4*right.m3 + left.m5*right.m7 + left.m6*right.m11 + left.m7*right.m15;
    result.m8 = left.m8*right.m0 + left.m9*right.m4 + left.m10*right.m8 + left.m11*right.m12;
    result.m9 = left.m8*right.m1 + left.m9*right.m5 + left.m10*right.m9 + left.m11*right.m13;
    result.m10 = left.m8*right.m2 + left.m9*right.m6 + left.m10*right.m10 + left.m11*right.m14;
    result.m11 = left.m8*right.m3 + left.m9*right.m7 + left.m10*right.m11 + left.m11*right.m15;
    result.m12 = left.m12*right.m0 + left.m13*right.m4 + left.m14*right.m8 + left.m15*right.m12;
    result.m13 = left.m12*right.m1 + left.m13*right.m5 + left.m14*right.m9 + left.m15*right.m13;
    result.m14 = left.m12*right.m2 + left.m13*right.m6 + left.m14*right.m10 + left.m15*right.m14;
    result.m15 = left.m12*right.m3 + left.m13*right.m7 + left.m14*right.m11 + left.m15*right.m15;

    return result;
}

// Get translation matrix
RMSAPI Matrix MatrixTranslate(f32 x, f32 y, f32 z)
{
    Matrix result = { 1.0f, 0.0f, 0.0f, x,
                      0.0f, 1.0f, 0.0f, y,
                      0.0f, 0.0f, 1.0f, z,
                      0.0f, 0.0f, 0.0f, 1.0f };

    return result;
}

// Create rotation matrix from axis and angle
// NOTE: Angle should be provided in radians
RMSAPI Matrix MatrixRotate(V3 axis, f32 angle)
{
    Matrix result = { 0 };

    f32 x = axis.x, y = axis.y, z = axis.z;

    f32 lengthSquared = x*x + y*y + z*z;

    if ((lengthSquared != 1.0f) && (lengthSquared != 0.0f))
    {
        f32 ilength = 1.0f/sqrtf(lengthSquared);
        x *= ilength;
        y *= ilength;
        z *= ilength;
    }

    f32 sinres = sinf(angle);
    f32 cosres = cosf(angle);
    f32 t = 1.0f - cosres;

    result.m0 = x*x*t + cosres;
    result.m1 = y*x*t + z*sinres;
    result.m2 = z*x*t - y*sinres;
    result.m3 = 0.0f;

    result.m4 = x*y*t - z*sinres;
    result.m5 = y*y*t + cosres;
    result.m6 = z*y*t + x*sinres;
    result.m7 = 0.0f;

    result.m8 = x*z*t + y*sinres;
    result.m9 = y*z*t - x*sinres;
    result.m10 = z*z*t + cosres;
    result.m11 = 0.0f;

    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = 0.0f;
    result.m15 = 1.0f;

    return result;
}

// Get x-rotation matrix
// NOTE: Angle must be provided in radians
RMSAPI Matrix MatrixRotateX(f32 angle)
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f }; // MatrixIdentity()

    f32 cosres = cosf(angle);
    f32 sinres = sinf(angle);

    result.m5 = cosres;
    result.m6 = sinres;
    result.m9 = -sinres;
    result.m10 = cosres;

    return result;
}

// Get y-rotation matrix
// NOTE: Angle must be provided in radians
RMSAPI Matrix MatrixRotateY(f32 angle)
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f }; // MatrixIdentity()

    f32 cosres = cosf(angle);
    f32 sinres = sinf(angle);

    result.m0 = cosres;
    result.m2 = -sinres;
    result.m8 = sinres;
    result.m10 = cosres;

    return result;
}

// Get z-rotation matrix
// NOTE: Angle must be provided in radians
RMSAPI Matrix MatrixRotateZ(f32 angle)
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f }; // MatrixIdentity()

    f32 cosres = cosf(angle);
    f32 sinres = sinf(angle);

    result.m0 = cosres;
    result.m1 = sinres;
    result.m4 = -sinres;
    result.m5 = cosres;

    return result;
}


// Get xyz-rotation matrix
// NOTE: Angle must be provided in radians
RMSAPI Matrix MatrixRotateXYZ(V3 angle)
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f }; // MatrixIdentity()

    f32 cosz = cosf(-angle.z);
    f32 sinz = sinf(-angle.z);
    f32 cosy = cosf(-angle.y);
    f32 siny = sinf(-angle.y);
    f32 cosx = cosf(-angle.x);
    f32 sinx = sinf(-angle.x);

    result.m0 = cosz*cosy;
    result.m1 = (cosz*siny*sinx) - (sinz*cosx);
    result.m2 = (cosz*siny*cosx) + (sinz*sinx);

    result.m4 = sinz*cosy;
    result.m5 = (sinz*siny*sinx) + (cosz*cosx);
    result.m6 = (sinz*siny*cosx) - (cosz*sinx);

    result.m8 = -siny;
    result.m9 = cosy*sinx;
    result.m10= cosy*cosx;

    return result;
}

// Get zyx-rotation matrix
// NOTE: Angle must be provided in radians
RMSAPI Matrix MatrixRotateZYX(V3 angle)
{
    Matrix result = { 0 };

    f32 cz = cosf(angle.z);
    f32 sz = sinf(angle.z);
    f32 cy = cosf(angle.y);
    f32 sy = sinf(angle.y);
    f32 cx = cosf(angle.x);
    f32 sx = sinf(angle.x);

    result.m0 = cz*cy;
    result.m4 = cz*sy*sx - cx*sz;
    result.m8 = sz*sx + cz*cx*sy;
    result.m12 = 0;

    result.m1 = cy*sz;
    result.m5 = cz*cx + sz*sy*sx;
    result.m9 = cx*sz*sy - cz*sx;
    result.m13 = 0;

    result.m2 = -sy;
    result.m6 = cy*sx;
    result.m10 = cy*cx;
    result.m14 = 0;

    result.m3 = 0;
    result.m7 = 0;
    result.m11 = 0;
    result.m15 = 1;

    return result;
}

// Get scaling matrix
RMSAPI Matrix MatrixScale(f32 x, f32 y, f32 z)
{
    Matrix result = { x, 0.0f, 0.0f, 0.0f,
                      0.0f, y, 0.0f, 0.0f,
                      0.0f, 0.0f, z, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f };

    return result;
}

// Get perspective projection matrix
RMSAPI Matrix MatrixFrustum(double left, double right, double bottom, double top, double near, double far)
{
    Matrix result = { 0 };

    f32 rl = (f32)(right - left);
    f32 tb = (f32)(top - bottom);
    f32 fn = (f32)(far - near);

    result.m0 = ((f32)near*2.0f)/rl;
    result.m1 = 0.0f;
    result.m2 = 0.0f;
    result.m3 = 0.0f;

    result.m4 = 0.0f;
    result.m5 = ((f32)near*2.0f)/tb;
    result.m6 = 0.0f;
    result.m7 = 0.0f;

    result.m8 = ((f32)right + (f32)left)/rl;
    result.m9 = ((f32)top + (f32)bottom)/tb;
    result.m10 = -((f32)far + (f32)near)/fn;
    result.m11 = -1.0f;

    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = -((f32)far*(f32)near*2.0f)/fn;
    result.m15 = 0.0f;

    return result;
}

// Get perspective projection matrix
// NOTE: Fovy angle must be provided in radians
RMSAPI Matrix MatrixPerspective(double fovY, double aspect, double nearPlane, double farPlane)
{
    Matrix result = { 0 };

    double top = nearPlane*tan(fovY*0.5);
    double bottom = -top;
    double right = top*aspect;
    double left = -right;

    // MatrixFrustum(-right, right, -top, top, near, far);
    f32 rl = (f32)(right - left);
    f32 tb = (f32)(top - bottom);
    f32 fn = (f32)(farPlane - nearPlane);

    result.m0 = ((f32)nearPlane*2.0f)/rl;
    result.m5 = ((f32)nearPlane*2.0f)/tb;
    result.m8 = ((f32)right + (f32)left)/rl;
    result.m9 = ((f32)top + (f32)bottom)/tb;
    result.m10 = -((f32)farPlane + (f32)nearPlane)/fn;
    result.m11 = -1.0f;
    result.m14 = -((f32)farPlane*(f32)nearPlane*2.0f)/fn;

    return result;
}

// Get orthographic projection matrix
RMSAPI Matrix MatrixOrtho(double left, double right, double bottom, double top, double nearPlane, double farPlane)
{
    Matrix result = { 0 };

    f32 rl = (f32)(right - left);
    f32 tb = (f32)(top - bottom);
    f32 fn = (f32)(farPlane - nearPlane);

    result.m0 = 2.0f/rl;
    result.m1 = 0.0f;
    result.m2 = 0.0f;
    result.m3 = 0.0f;
    result.m4 = 0.0f;
    result.m5 = 2.0f/tb;
    result.m6 = 0.0f;
    result.m7 = 0.0f;
    result.m8 = 0.0f;
    result.m9 = 0.0f;
    result.m10 = -2.0f/fn;
    result.m11 = 0.0f;
    result.m12 = -((f32)left + (f32)right)/rl;
    result.m13 = -((f32)top + (f32)bottom)/tb;
    result.m14 = -((f32)farPlane + (f32)nearPlane)/fn;
    result.m15 = 1.0f;

    return result;
}

// Get camera look-at matrix (view matrix)
RMSAPI Matrix MatrixLookAt(V3 eye, V3 target, V3 up)
{
    Matrix result = { 0 };

    f32 length = 0.0f;
    f32 ilength = 0.0f;

    // V3Subtract(eye, target)
    V3 vz = { eye.x - target.x, eye.y - target.y, eye.z - target.z };

    // V3Normalize(vz)
    V3 v = vz;
    length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f/length;
    vz.x *= ilength;
    vz.y *= ilength;
    vz.z *= ilength;

    // V3CrossProduct(up, vz)
    V3 vx = { up.y*vz.z - up.z*vz.y, up.z*vz.x - up.x*vz.z, up.x*vz.y - up.y*vz.x };

    // V3Normalize(x)
    v = vx;
    length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f/length;
    vx.x *= ilength;
    vx.y *= ilength;
    vx.z *= ilength;

    // V3CrossProduct(vz, vx)
    V3 vy = { vz.y*vx.z - vz.z*vx.y, vz.z*vx.x - vz.x*vx.z, vz.x*vx.y - vz.y*vx.x };

    result.m0 = vx.x;
    result.m1 = vy.x;
    result.m2 = vz.x;
    result.m3 = 0.0f;
    result.m4 = vx.y;
    result.m5 = vy.y;
    result.m6 = vz.y;
    result.m7 = 0.0f;
    result.m8 = vx.z;
    result.m9 = vy.z;
    result.m10 = vz.z;
    result.m11 = 0.0f;
    result.m12 = -(vx.x*eye.x + vx.y*eye.y + vx.z*eye.z);   // V3DotProduct(vx, eye)
    result.m13 = -(vy.x*eye.x + vy.y*eye.y + vy.z*eye.z);   // V3DotProduct(vy, eye)
    result.m14 = -(vz.x*eye.x + vz.y*eye.y + vz.z*eye.z);   // V3DotProduct(vz, eye)
    result.m15 = 1.0f;

    return result;
}

// Get f32 array of matrix data
RMSAPI f32_16 MatrixTof32V(Matrix mat)
{
    f32_16 result = { 0 };

    result.v[0] = mat.m0;
    result.v[1] = mat.m1;
    result.v[2] = mat.m2;
    result.v[3] = mat.m3;
    result.v[4] = mat.m4;
    result.v[5] = mat.m5;
    result.v[6] = mat.m6;
    result.v[7] = mat.m7;
    result.v[8] = mat.m8;
    result.v[9] = mat.m9;
    result.v[10] = mat.m10;
    result.v[11] = mat.m11;
    result.v[12] = mat.m12;
    result.v[13] = mat.m13;
    result.v[14] = mat.m14;
    result.v[15] = mat.m15;

    return result;
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Quaternion math
//----------------------------------------------------------------------------------

// Add two quaternions
RMSAPI Quaternion QuaternionAdd(Quaternion q1, Quaternion q2)
{
    Quaternion result = {q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w};

    return result;
}

// Add quaternion and f32 value
RMSAPI Quaternion QuaternionAddValue(Quaternion q, f32 add)
{
    Quaternion result = {q.x + add, q.y + add, q.z + add, q.w + add};

    return result;
}

// Subtract two quaternions
RMSAPI Quaternion QuaternionSubtract(Quaternion q1, Quaternion q2)
{
    Quaternion result = {q1.x - q2.x, q1.y - q2.y, q1.z - q2.z, q1.w - q2.w};

    return result;
}

// Subtract quaternion and f32 value
RMSAPI Quaternion QuaternionSubtractValue(Quaternion q, f32 sub)
{
    Quaternion result = {q.x - sub, q.y - sub, q.z - sub, q.w - sub};

    return result;
}

// Get identity quaternion
RMSAPI Quaternion QuaternionIdentity(void)
{
    Quaternion result = { 0.0f, 0.0f, 0.0f, 1.0f };

    return result;
}

// Computes the length of a quaternion
RMSAPI f32 QuaternionLength(Quaternion q)
{
    f32 result = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);

    return result;
}

// Normalize provided quaternion
RMSAPI Quaternion QuaternionNormalize(Quaternion q)
{
    Quaternion result = { 0 };

    f32 length = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
    if (length == 0.0f) length = 1.0f;
    f32 ilength = 1.0f/length;

    result.x = q.x*ilength;
    result.y = q.y*ilength;
    result.z = q.z*ilength;
    result.w = q.w*ilength;

    return result;
}

// Invert provided quaternion
RMSAPI Quaternion QuaternionInvert(Quaternion q)
{
    Quaternion result = q;

    f32 lengthSq = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w;

    if (lengthSq != 0.0f)
    {
        f32 invLength = 1.0f/lengthSq;

        result.x *= -invLength;
        result.y *= -invLength;
        result.z *= -invLength;
        result.w *= invLength;
    }

    return result;
}

// Calculate two quaternion multiplication
RMSAPI Quaternion QuaternionMultiply(Quaternion q1, Quaternion q2)
{
    Quaternion result = { 0 };

    f32 qax = q1.x, qay = q1.y, qaz = q1.z, qaw = q1.w;
    f32 qbx = q2.x, qby = q2.y, qbz = q2.z, qbw = q2.w;

    result.x = qax*qbw + qaw*qbx + qay*qbz - qaz*qby;
    result.y = qay*qbw + qaw*qby + qaz*qbx - qax*qbz;
    result.z = qaz*qbw + qaw*qbz + qax*qby - qay*qbx;
    result.w = qaw*qbw - qax*qbx - qay*qby - qaz*qbz;

    return result;
}

// Scale quaternion by f32 value
RMSAPI Quaternion QuaternionScale(Quaternion q, f32 mul)
{
    Quaternion result = { 0 };

    result.x = q.x*mul;
    result.y = q.y*mul;
    result.z = q.z*mul;
    result.w = q.w*mul;

    return result;
}

// Divide two quaternions
RMSAPI Quaternion QuaternionDivide(Quaternion q1, Quaternion q2)
{
    Quaternion result = { q1.x/q2.x, q1.y/q2.y, q1.z/q2.z, q1.w/q2.w };

    return result;
}

// Calculate linear interpolation between two quaternions
RMSAPI Quaternion QuaternionLerp(Quaternion q1, Quaternion q2, f32 amount)
{
    Quaternion result = { 0 };

    result.x = q1.x + amount*(q2.x - q1.x);
    result.y = q1.y + amount*(q2.y - q1.y);
    result.z = q1.z + amount*(q2.z - q1.z);
    result.w = q1.w + amount*(q2.w - q1.w);

    return result;
}

// Calculate slerp-optimized interpolation between two quaternions
RMSAPI Quaternion QuaternionNlerp(Quaternion q1, Quaternion q2, f32 amount)
{
    Quaternion result = { 0 };

    // QuaternionLerp(q1, q2, amount)
    result.x = q1.x + amount*(q2.x - q1.x);
    result.y = q1.y + amount*(q2.y - q1.y);
    result.z = q1.z + amount*(q2.z - q1.z);
    result.w = q1.w + amount*(q2.w - q1.w);

    // QuaternionNormalize(q);
    Quaternion q = result;
    f32 length = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
    if (length == 0.0f) length = 1.0f;
    f32 ilength = 1.0f/length;

    result.x = q.x*ilength;
    result.y = q.y*ilength;
    result.z = q.z*ilength;
    result.w = q.w*ilength;

    return result;
}

// Calculates spherical linear interpolation between two quaternions
RMSAPI Quaternion QuaternionSlerp(Quaternion q1, Quaternion q2, f32 amount)
{
    Quaternion result = { 0 };

#if !defined(EPSILON)
    #define EPSILON 0.000001f
#endif

    f32 cosHalfTheta = q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;

    if (cosHalfTheta < 0)
    {
        q2.x = -q2.x; q2.y = -q2.y; q2.z = -q2.z; q2.w = -q2.w;
        cosHalfTheta = -cosHalfTheta;
    }

    if (fabsf(cosHalfTheta) >= 1.0f) result = q1;
    else if (cosHalfTheta > 0.95f) result = QuaternionNlerp(q1, q2, amount);
    else
    {
        f32 halfTheta = acosf(cosHalfTheta);
        f32 sinHalfTheta = sqrtf(1.0f - cosHalfTheta*cosHalfTheta);

        if (fabsf(sinHalfTheta) < EPSILON)
        {
            result.x = (q1.x*0.5f + q2.x*0.5f);
            result.y = (q1.y*0.5f + q2.y*0.5f);
            result.z = (q1.z*0.5f + q2.z*0.5f);
            result.w = (q1.w*0.5f + q2.w*0.5f);
        }
        else
        {
            f32 ratioA = sinf((1 - amount)*halfTheta)/sinHalfTheta;
            f32 ratioB = sinf(amount*halfTheta)/sinHalfTheta;

            result.x = (q1.x*ratioA + q2.x*ratioB);
            result.y = (q1.y*ratioA + q2.y*ratioB);
            result.z = (q1.z*ratioA + q2.z*ratioB);
            result.w = (q1.w*ratioA + q2.w*ratioB);
        }
    }

    return result;
}

// Calculate quaternion based on the rotation from one vector to another
RMSAPI Quaternion QuaternionFromV3ToV3(V3 from, V3 to)
{
    Quaternion result = { 0 };

    f32 cos2Theta = (from.x*to.x + from.y*to.y + from.z*to.z);    // V3DotProduct(from, to)
    V3 cross = { from.y*to.z - from.z*to.y, from.z*to.x - from.x*to.z, from.x*to.y - from.y*to.x }; // V3CrossProduct(from, to)

    result.x = cross.x;
    result.y = cross.y;
    result.z = cross.z;
    result.w = 1.0f + cos2Theta;

    // QuaternionNormalize(q);
    // NOTE: Normalize to essentially nlerp the original and identity to 0.5
    Quaternion q = result;
    f32 length = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
    if (length == 0.0f) length = 1.0f;
    f32 ilength = 1.0f/length;

    result.x = q.x*ilength;
    result.y = q.y*ilength;
    result.z = q.z*ilength;
    result.w = q.w*ilength;

    return result;
}

// Get a quaternion for a given rotation matrix
RMSAPI Quaternion QuaternionFromMatrix(Matrix mat)
{
    Quaternion result = { 0 };

    f32 fourWSquaredMinus1 = mat.m0  + mat.m5 + mat.m10;
    f32 fourXSquaredMinus1 = mat.m0  - mat.m5 - mat.m10;
    f32 fourYSquaredMinus1 = mat.m5  - mat.m0 - mat.m10;
    f32 fourZSquaredMinus1 = mat.m10 - mat.m0 - mat.m5;

    int biggestIndex = 0;
    f32 fourBiggestSquaredMinus1 = fourWSquaredMinus1;
    if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
    {
        fourBiggestSquaredMinus1 = fourXSquaredMinus1;
        biggestIndex = 1;
    }

    if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
    {
        fourBiggestSquaredMinus1 = fourYSquaredMinus1;
        biggestIndex = 2;
    }

    if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
    {
        fourBiggestSquaredMinus1 = fourZSquaredMinus1;
        biggestIndex = 3;
    }

    f32 biggestVal = sqrtf(fourBiggestSquaredMinus1 + 1.0f)*0.5f;
    f32 mult = 0.25f / biggestVal;

    switch (biggestIndex)
    {
        case 0:
            result.w = biggestVal;
            result.x = (mat.m6 - mat.m9)*mult;
            result.y = (mat.m8 - mat.m2)*mult;
            result.z = (mat.m1 - mat.m4)*mult;
            break;
        case 1:
            result.x = biggestVal;
            result.w = (mat.m6 - mat.m9)*mult;
            result.y = (mat.m1 + mat.m4)*mult;
            result.z = (mat.m8 + mat.m2)*mult;
            break;
        case 2:
            result.y = biggestVal;
            result.w = (mat.m8 - mat.m2)*mult;
            result.x = (mat.m1 + mat.m4)*mult;
            result.z = (mat.m6 + mat.m9)*mult;
            break;
        case 3:
            result.z = biggestVal;
            result.w = (mat.m1 - mat.m4)*mult;
            result.x = (mat.m8 + mat.m2)*mult;
            result.y = (mat.m6 + mat.m9)*mult;
            break;
    }

    return result;
}

// Get a matrix for a given quaternion
RMSAPI Matrix QuaternionToMatrix(Quaternion q)
{
    Matrix result = { 1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f }; // MatrixIdentity()

    f32 a2 = q.x*q.x;
    f32 b2 = q.y*q.y;
    f32 c2 = q.z*q.z;
    f32 ac = q.x*q.z;
    f32 ab = q.x*q.y;
    f32 bc = q.y*q.z;
    f32 ad = q.w*q.x;
    f32 bd = q.w*q.y;
    f32 cd = q.w*q.z;

    result.m0 = 1 - 2*(b2 + c2);
    result.m1 = 2*(ab + cd);
    result.m2 = 2*(ac - bd);

    result.m4 = 2*(ab - cd);
    result.m5 = 1 - 2*(a2 + c2);
    result.m6 = 2*(bc + ad);

    result.m8 = 2*(ac + bd);
    result.m9 = 2*(bc - ad);
    result.m10 = 1 - 2*(a2 + b2);

    return result;
}

// Get rotation quaternion for an angle and axis
// NOTE: Angle must be provided in radians
RMSAPI Quaternion QuaternionFromAxisAngle(V3 axis, f32 angle)
{
    Quaternion result = { 0.0f, 0.0f, 0.0f, 1.0f };

    f32 axisLength = sqrtf(axis.x*axis.x + axis.y*axis.y + axis.z*axis.z);

    if (axisLength != 0.0f)
    {
        angle *= 0.5f;

        f32 length = 0.0f;
        f32 ilength = 0.0f;

        // V3Normalize(axis)
        V3 v = axis;
        length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
        if (length == 0.0f) length = 1.0f;
        ilength = 1.0f/length;
        axis.x *= ilength;
        axis.y *= ilength;
        axis.z *= ilength;

        f32 sinres = sinf(angle);
        f32 cosres = cosf(angle);

        result.x = axis.x*sinres;
        result.y = axis.y*sinres;
        result.z = axis.z*sinres;
        result.w = cosres;

        // QuaternionNormalize(q);
        Quaternion q = result;
        length = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
        if (length == 0.0f) length = 1.0f;
        ilength = 1.0f/length;
        result.x = q.x*ilength;
        result.y = q.y*ilength;
        result.z = q.z*ilength;
        result.w = q.w*ilength;
    }

    return result;
}

// Get the rotation angle and axis for a given quaternion
RMSAPI void QuaternionToAxisAngle(Quaternion q, V3 *outAxis, f32 *outAngle)
{
    if (fabsf(q.w) > 1.0f)
    {
        // QuaternionNormalize(q);
        f32 length = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
        if (length == 0.0f) length = 1.0f;
        f32 ilength = 1.0f/length;

        q.x = q.x*ilength;
        q.y = q.y*ilength;
        q.z = q.z*ilength;
        q.w = q.w*ilength;
    }

    V3 resAxis = { 0.0f, 0.0f, 0.0f };
    f32 resAngle = 2.0f*acosf(q.w);
    f32 den = sqrtf(1.0f - q.w*q.w);

    if (den > EPSILON)
    {
        resAxis.x = q.x/den;
        resAxis.y = q.y/den;
        resAxis.z = q.z/den;
    }
    else
    {
        // This occurs when the angle is zero.
        // Not a problem: just set an arbitrary normalized axis.
        resAxis.x = 1.0f;
    }

    *outAxis = resAxis;
    *outAngle = resAngle;
}

// Get the quaternion equivalent to Euler angles
// NOTE: Rotation order is ZYX
RMSAPI Quaternion QuaternionFromEuler(f32 pitch, f32 yaw, f32 roll)
{
    Quaternion result = { 0 };

    f32 x0 = cosf(pitch*0.5f);
    f32 x1 = sinf(pitch*0.5f);
    f32 y0 = cosf(yaw*0.5f);
    f32 y1 = sinf(yaw*0.5f);
    f32 z0 = cosf(roll*0.5f);
    f32 z1 = sinf(roll*0.5f);

    result.x = x1*y0*z0 - x0*y1*z1;
    result.y = x0*y1*z0 + x1*y0*z1;
    result.z = x0*y0*z1 - x1*y1*z0;
    result.w = x0*y0*z0 + x1*y1*z1;

    return result;
}

// Get the Euler angles equivalent to quaternion (roll, pitch, yaw)
// NOTE: Angles are returned in a V3 struct in radians
RMSAPI V3 QuaternionToEuler(Quaternion q)
{
    V3 result = { 0 };

    // Roll (x-axis rotation)
    f32 x0 = 2.0f*(q.w*q.x + q.y*q.z);
    f32 x1 = 1.0f - 2.0f*(q.x*q.x + q.y*q.y);
    result.x = atan2f(x0, x1);

    // Pitch (y-axis rotation)
    f32 y0 = 2.0f*(q.w*q.y - q.z*q.x);
    y0 = y0 > 1.0f ? 1.0f : y0;
    y0 = y0 < -1.0f ? -1.0f : y0;
    result.y = asinf(y0);

    // Yaw (z-axis rotation)
    f32 z0 = 2.0f*(q.w*q.z + q.x*q.y);
    f32 z1 = 1.0f - 2.0f*(q.y*q.y + q.z*q.z);
    result.z = atan2f(z0, z1);

    return result;
}

// Transform a quaternion given a transformation matrix
RMSAPI Quaternion QuaternionTransform(Quaternion q, Matrix mat)
{
    Quaternion result = { 0 };

    result.x = mat.m0*q.x + mat.m4*q.y + mat.m8*q.z + mat.m12*q.w;
    result.y = mat.m1*q.x + mat.m5*q.y + mat.m9*q.z + mat.m13*q.w;
    result.z = mat.m2*q.x + mat.m6*q.y + mat.m10*q.z + mat.m14*q.w;
    result.w = mat.m3*q.x + mat.m7*q.y + mat.m11*q.z + mat.m15*q.w;

    return result;
}

// Check whether two given quaternions are almost equal
RMSAPI int QuaternionEquals(Quaternion p, Quaternion q)
{
#if !defined(EPSILON)
    #define EPSILON 0.000001f
#endif

    int result = (((fabsf(p.x - q.x)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.x), fabsf(q.x))))) &&
                  ((fabsf(p.y - q.y)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.y), fabsf(q.y))))) &&
                  ((fabsf(p.z - q.z)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.z), fabsf(q.z))))) &&
                  ((fabsf(p.w - q.w)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.w), fabsf(q.w)))))) ||
                 (((fabsf(p.x + q.x)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.x), fabsf(q.x))))) &&
                  ((fabsf(p.y + q.y)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.y), fabsf(q.y))))) &&
                  ((fabsf(p.z + q.z)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.z), fabsf(q.z))))) &&
                  ((fabsf(p.w + q.w)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.w), fabsf(q.w))))));

    return result;
}

#endif // 

#endif
