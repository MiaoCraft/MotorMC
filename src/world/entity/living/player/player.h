#pragma once

#include "player.d.h"

#include "../../../../main.h"
#include "../../../../jobs/scheduler/scheduler.h"
#include "../living.h"

struct ent_player {

	ent_living_entity_t living_entity;
	
	const byte_t* uuid;

	float32_t additional_hearts;
	int32_t score;
	
	uint32_t digging;

	_Atomic byte_t displayed_skin_parts;

	uint8_t held_item : 4;
	byte_t main_hand : 1;

	ent_gamemode_t gamemode : 2;

	bool digging_block : 1;

	itm_item_t inventory[27];
	itm_item_t hotbar[9];
	itm_item_t carried;

	struct {
		itm_item_t* elements[90];
		uint8_t count;
	} paint;

	// for parrots
	ent_living_entity_t* left_shoulder;
	ent_living_entity_t* right_shoulder;

};

static inline ent_living_entity_t* ent_player_get_living_entity(ent_player_t* player) {
	return &player->living_entity;
}

static inline ent_entity_t* ent_player_get_entity(ent_player_t* player) {
	return &player->living_entity.entity;
}

// player specific functions
static inline bool ent_player_is_best_tool(const mat_block_t* block, const mat_item_t* item) {

	if (block->mineable) {

		if (item->tool) {

			if (
				(block->mineable_axe && item->tool_axe) ||
				(block->mineable_hoe && item->tool_hoe) ||
				(block->mineable_pickaxe && item->tool_pickaxe) ||
				(block->mineable_shovel && item->tool_shovel)
			) {
				return true;
			}

		}

		return false;
	}

	return true;

}

static inline bool ent_player_can_harvest(const mat_block_t* block, const mat_item_t* item) {

	if (block->needs) {

		if (item->tool) {

			if (
				(block->needs_wooden_tool && item->least_wooden_tool) ||
				(block->needs_stone_tool && item->least_stone_tool) ||
				(block->needs_iron_tool && item->least_iron_tool) ||
				(block->needs_diamond_tool && item->least_diamond_tool)
			) {
				return true;
			}

		}

		return false;

	}

	return true;

}

extern uint16_t ent_player_get_break_speed(ent_player_t* player, mat_block_type_t block);

static inline bool ent_player_is_digging_block(ent_player_t* player) {

	bool digging;

	with_lock (&ent_player_get_entity(player)->lock) {
		digging = player->digging_block;
	}

	return digging;

}

static inline void ent_player_stop_digging_block(ent_player_t* player) {

	with_lock (&ent_player_get_entity(player)->lock) {
		player->digging_block = false;
	}

}

static inline void ent_player_start_digging_block(ent_player_t* player, uint32_t job) {

	with_lock (&ent_player_get_entity(player)->lock) {
		player->digging_block = true;
		player->digging = job;
	}

}

static inline void ent_player_cancel_digging_block(ent_player_t* player) {
	
	with_lock (&ent_player_get_entity(player)->lock) {
		sch_cancel(player->digging);
		player->digging_block = false;
	}

}

static inline void ent_player_set_displayed_skin_parts(ent_player_t* player, byte_t displayed_skin_parts) {
	
	player->displayed_skin_parts = displayed_skin_parts;

}

static inline void ent_player_set_main_hand(ent_player_t* player, byte_t main_hand) {

	with_lock (&ent_player_get_entity(player)->lock) {
		player->main_hand = main_hand;
	}

}

static inline void ent_player_set_held_item(ent_player_t* player, uint16_t held_item) {

	player->held_item = held_item;

}

extern void ent_free_player(ent_player_t* entity);