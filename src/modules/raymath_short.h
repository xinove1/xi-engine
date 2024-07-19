#ifndef RAYMATH_SHORT_H_
# define RAYMATH_SHORT_H_
# include "types.h"
# include <math.h>

#ifdef RAYMATH_SHORT_IMPLEMENTATION
# define RMSAPI extern inline
#else
# define RMSAPI inline
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition - V2 math
//----------------------------------------------------------------------------------

// Vector with components value 0.0f
RMSAPI V2 V2Zero(void)
{
    V2 result = { 0.0f, 0.0f };

    return result;
}

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

// Add vector and float value
RMSAPI V2 V2AddValue(V2 v, float add)
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

// Subtract vector by float value
RMSAPI V2 V2SubtractValue(V2 v, float sub)
{
    V2 result = { v.x - sub, v.y - sub };

    return result;
}

// Calculate vector length
RMSAPI float V2Length(V2 v)
{
    float result = sqrtf((v.x*v.x) + (v.y*v.y));

    return result;
}

// Calculate vector square length
RMSAPI float V2LengthSqr(V2 v)
{
    float result = (v.x*v.x) + (v.y*v.y);

    return result;
}

// Calculate two vectors dot product
RMSAPI float V2DotProduct(V2 v1, V2 v2)
{
    float result = (v1.x*v2.x + v1.y*v2.y);

    return result;
}

// Calculate distance between two vectors
RMSAPI float V2Distance(V2 v1, V2 v2)
{
    float result = sqrtf((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y));

    return result;
}

// Calculate square distance between two vectors
RMSAPI float V2DistanceSqr(V2 v1, V2 v2)
{
    float result = ((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y));

    return result;
}

// Calculate angle between two vectors
// NOTE: Angle is calculated from origin point (0, 0)
RMSAPI float V2Angle(V2 v1, V2 v2)
{
    float result = 0.0f;

    float dot = v1.x*v2.x + v1.y*v2.y;
    float det = v1.x*v2.y - v1.y*v2.x;

    result = atan2f(det, dot);

    return result;
}

// Calculate angle defined by a two vectors line
// NOTE: Parameters need to be normalized
// Current implementation should be aligned with glm::angle
RMSAPI float V2LineAngle(V2 start, V2 end)
{
    float result = 0.0f;

    // TODO(10/9/2023): Currently angles move clockwise, determine if this is wanted behavior
    result = -atan2f(end.y - start.y, end.x - start.x);

    return result;
}

// Scale vector (multiply by value)
RMSAPI V2 V2Scale(V2 v, float scale)
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
    float length = sqrtf((v.x*v.x) + (v.y*v.y));

    if (length > 0)
    {
        float ilength = 1.0f/length;
        result.x = v.x*ilength;
        result.y = v.y*ilength;
    }

    return result;
}

// Transforms a V2 by a given Matrix
RMSAPI V2 V2Transform(V2 v, Matrix mat)
{
    V2 result = { 0 };

    float x = v.x;
    float y = v.y;
    float z = 0;

    result.x = mat.m0*x + mat.m4*y + mat.m8*z + mat.m12;
    result.y = mat.m1*x + mat.m5*y + mat.m9*z + mat.m13;

    return result;
}

// Calculate linear interpolation between two vectors
RMSAPI V2 V2Lerp(V2 v1, V2 v2, float amount)
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

    float dotProduct = (v.x*normal.x + v.y*normal.y); // Dot product

    result.x = v.x - (2.0f*normal.x)*dotProduct;
    result.y = v.y - (2.0f*normal.y)*dotProduct;

    return result;
}

// Rotate vector by angle
RMSAPI V2 V2Rotate(V2 v, float angle)
{
    V2 result = { 0 };

    float cosres = cosf(angle);
    float sinres = sinf(angle);

    result.x = v.x*cosres - v.y*sinres;
    result.y = v.x*sinres + v.y*cosres;

    return result;
}

// Move Vector towards target
RMSAPI V2 V2MoveTowards(V2 v, V2 target, float maxDistance)
{
    V2 result = { 0 };

    float dx = target.x - v.x;
    float dy = target.y - v.y;
    float value = (dx*dx) + (dy*dy);

    if ((value == 0) || ((maxDistance >= 0) && (value <= maxDistance*maxDistance))) return target;

    float dist = sqrtf(value);

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
RMSAPI V2 V2ClampValue(V2 v, float min, float max)
{
    V2 result = v;

    float length = (v.x*v.x) + (v.y*v.y);
    if (length > 0.0f)
    {
        length = sqrtf(length);

        if (length < min)
        {
            float scale = min/length;
            result.x = v.x*scale;
            result.y = v.y*scale;
        }
        else if (length > max)
        {
            float scale = max/length;
            result.x = v.x*scale;
            result.y = v.y*scale;
        }
    }

    return result;
}

// Check whether two given vectors are almost equal
RMSAPI int V2Equals(V2 p, V2 q)
{
#if !defined(EPSILON)
    #define EPSILON 0.000001f
#endif

    int result = ((fabsf(p.x - q.x)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.x), fabsf(q.x))))) &&
                  ((fabsf(p.y - q.y)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.y), fabsf(q.y)))));

    return result;
}

#endif
