#ifndef COLLISION_H_
# define COLLISION_H_
# include "raylib.h"
# include "raymath_short.h"
# include "core.h"

#endif

// b32 CheckCollisionRayRec(V2 origin, V2 dir, Rect rec,
// 			  V2 *contact_point, V2 *contact_normal, f32 *contact_time);
b32 CheckCollisionRayRec(V2 origin, V2 dir, Rect rec, V2 *contact_point, V2 *contact_normal, f32 *t_hit_near);
// b32 CheckCollisionDynamicRectRect(Rect rec, V2 vel, Rect target,
// 				   V2 *contact_point, V2 *contact_normal, f32 *contact_time, f32 delta_time);
b32 CheckCollisionDynamicRectRect(V2 origin, Rect rec, V2 vel, Rect target, V2 *contact_point, V2 *contact_normal, f32 *contact_time, f32 delta_time);

#ifdef XI_COLLISION_IMPLEMENTATION

// NOLINTBEGIN(misc-definitions-in-headers)
b32 CheckCollisionRayRec(V2 origin, V2 dir, Rect rec, V2 *contact_point, V2 *contact_normal, f32 *t_hit_near)
{
	V2 rec_pos = {rec.x, rec.y};
	V2 rec_pos_side = {rec.x + rec.width, rec.y + rec.height};
	V2 t_near = V2Divide(V2Subtract(rec_pos, origin), dir);
	V2 t_far = V2Divide(V2Subtract(rec_pos_side, origin), dir);

	// if (dir.x == 0 && dir.y == 0) {
	// 	return (false);
	// }

	if (t_near.x > t_far.x)
	{
		f32 tmp = t_near.x;
		t_near.x = t_far.x;
		t_far.x = tmp;
	}
	if (t_near.y > t_far.y)
	{
		f32 tmp = t_near.y;
		t_near.y = t_far.y;
		t_far.y = tmp;
	}

	if (t_near.x > t_far.y || t_near.y > t_far.x)
		return (false);

	*t_hit_near = fmaxf(t_near.x, t_near.y);
	f32 t_hit_far = fminf(t_far.x, t_far.y);

	if (t_hit_far < 0)
		return (false);

	*contact_point = V2Add(origin, V2Scale(dir, *t_hit_near));

	if (t_near.x > t_near.y)
	{
		if (dir.x < 0)
			*contact_normal = (Vector2) {1, 0};
		else
			*contact_normal = (Vector2) {-1, 0};
	}
	else if (t_near.x < t_near.y)
	{
		if (dir.y < 0)
			*contact_normal = (Vector2) {0, 1};
		else
			*contact_normal = (Vector2) {0, -1};
	}

	return (true);
}

b32 CheckCollisionDynamicRectRect(V2 origin, Rect rec, V2 vel, Rect target, V2 *contact_point, V2 *contact_normal, f32 *contact_time, f32 delta_time)
{
	if (vel.x == 0 && vel.y == 0) return (false);
	Rect extended_target;
	extended_target.x = target.x - rec.width/2;
	extended_target.y = target.y - rec.height/2;
	extended_target.width = target.width + rec.width;
	extended_target.height = target.height + rec.height;

	V2 rec_origin = {origin.x + rec.x, origin.y + rec.y};
	if (CheckCollisionRayRec(V2Add(rec_origin, (V2){rec.width * 0.5f, rec.height * 0.5f}), vel, extended_target, contact_point, contact_normal, contact_time))
	{
		if (*contact_time >= 0 && *contact_time < 1)
			return (true);
	}
	return (false);
}

// NOLINTEND(misc-definitions-in-headers)
#endif
