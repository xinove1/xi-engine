#include "game.h"

local void generate_packets(GameLevel *l)
{
	WaveManager *w = &l->wave_manager;
	w->packets_amount = 5;
	for (size i = 0; i < w->packets_amount; i++) {
		Entity enemy =  create_enemy_ex(Vec2v(0), (CreateEnemyParams) {
			.size = Vec2(16, 16),
			.health = 50,
			.color = BLUE,
			.speed = 70,
			.floor = 0,
			.melee = false,
			.damage = 1,
			.range = 10,
			.attack_rate = 0.5f
		});
		SpawnPacket packet = { .enemy = enemy, .cooldown = 1.3f };
		w->packets[i] = packet;
	}
}

local SpawnPacket get_next_packet(WaveManager *manager)
{
	SpawnPacket r = { 0 };
	if (manager->packet_current < manager->packets_amount) {
		r = manager->packets[manager->packet_current];
		manager->packet_current++;
	}
	return (r);
}

void update_wave_manager(GameLevel *l)
{
	WaveManager *w = &l->wave_manager;

	#ifdef BUILD_DEBUG
		if (w->time_until_next_wave == 0) {
			TraceLog(LOG_INFO, "wave manager time_until_next_wave is 0, no wait time until next wave.");
		} else if (w->time_until_next_wave < 0) {
			TraceLog(LOG_WARNING, "update_wave_manager: time_until_next_wave is negative, aborting.");
			return;
		}
		if (w->floor_limit <= 0) {
			TraceLog(LOG_INFO, "wave manager floor_limit is %d, won't spawn anything.", w->floor_limit);
		}
	#endif

	// If wave finished wait to start new one
	if (w->packet_current >= w->packets_amount) {
		
		// Returning early if there's still enemys alive
		if (w->time_count == 0) {
			{entitys_iterate(l->enemys) {
				Entity *e = iterate_get();
				if (e->type != EntityEmpty) return ;
			}}
		}

		w->time_count += GetFrameTime();
		if (w->time_count >= w->time_until_next_wave) {
			w->time_count = 0;
			w->packet_current = 0;
			w->wave++;
			if (w->wave % 2 == 0 && w->floor_limit < l->floors_count) { // Spawn on one more floor every 2 waves
				w->floor_limit++;
			}
			generate_packets(l);
		}
		return ;
	}

	// Consume packets
	for (size i = 0; i < w->floor_limit; i++) {
		for (size side = 0; side < 2; side++) {
			SpawnLocation *location = get_spawn_point(l, i, side ? right_side : left_side);
			location->cooldown -= GetFrameTime();
			if (location->cooldown <= 0) {
				SpawnPacket p = get_next_packet(w);
				p.enemy.pos = V2Subtract(location->point, V2Scale(p.enemy.size, 0.5f));
				p.enemy.floor = location->floor;
				push_entity(&l->enemys, p.enemy);
				location->cooldown = p.cooldown;
			}
		}
	}
}



SpawnLocation *get_spawn_point(GameLevel *l, i32 floor, i32 side)
{
	assert(floor < l->floors_count);
	if (side == right_side) {
		floor += l->floors_count;
	}
	return (&l->wave_manager.locations[floor]);
}
