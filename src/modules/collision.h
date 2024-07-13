#ifndef COLLISION_H_
# define COLLISION_H_
# include "raylib.h"
# include "raymath.h"
# include "types.h"

#endif

bool	CheckCollisionRayRec(Vector2 origin, Vector2 dir, Rectangle rec,
							 Vector2 *contact_point, Vector2 *contact_normal, float *contact_time);
bool	CheckCollisionDynamicRectRect(Rectangle rec, Vector2 vel, Rectangle target,
				      Vector2 *contact_point, Vector2 *contact_normal,
				      float *contact_time, float delta_time);

#ifdef XI_COLLISION_IMPLEMENTATION

// NOLINTBEGIN(misc-definitions-in-headers)
bool	CheckCollisionRayRec(Vector2 origin, Vector2 dir, Rectangle rec,
							 Vector2 *contact_point, Vector2 *contact_normal, float *contact_time)
{
	Vector2	rec_pos = {rec.x, rec.y};
	Vector2	rec_pos_side = {rec.x + rec.width, rec.y + rec.height};
	// NOTE Divide By zero could occour here and cause bugs
	Vector2	t_near = Vector2Divide(Vector2Subtract(rec_pos, origin), dir);
	Vector2	t_far = Vector2Divide(Vector2Subtract(rec_pos_side, origin), dir);

	if (t_near.x > t_far.x)
	{
		float	tmp = t_near.x;
		t_near.x = t_far.x;
		t_far.x = tmp;
	}
	if (t_near.y > t_far.y)
	{
		float	tmp = t_near.y;
		t_near.y = t_far.y;
		t_far.y = tmp;
	}

	if (t_near.x > t_far.y || t_near.y > t_far.x)
		return (false);

	*contact_time = fmaxf(t_near.x, t_near.y);
	float	t_hit_far = fminf(t_far.x, t_far.y);

	if (t_hit_far < 0)
		return (false);

	*contact_point = Vector2Add(origin, Vector2Scale(dir, *contact_time));

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

bool	CheckCollisionDynamicRectRect(Rectangle rec, Vector2 vel, Rectangle target,
				      Vector2 *contact_point, Vector2 *contact_normal,
				      float *contact_time, float delta_time)
{
	if (vel.x == 0 && vel.y == 0) {
		return (false);
	}
	Rectangle	extended_target;
	extended_target.x = target.x - rec.width/2;
	extended_target.y = target.y - rec.height/2;
	extended_target.width = target.width + rec.width;
	extended_target.height = target.height + rec.height;

	Vector2	origin = {rec.x, rec.y};
	if (CheckCollisionRayRec(Vector2Add(origin, (Vector2){rec.width/2, rec.height/2}), Vector2Scale(vel, delta_time), extended_target, contact_point, contact_normal, contact_time)) {
		if (*contact_time >= 0 && *contact_time < 1) {
			// To correct normal  from extended target
			*contact_point = Vector2Subtract(*contact_point, Vector2Multiply((V2) {rec.width/2, rec.height/2}, *contact_normal));
			return (true);
		}
	}
	return (false);
}

// NOLINTEND(misc-definitions-in-headers)
#endif
