/*
 * Copyright (C) 2015 NIPE-SYSTEMS
 * Copyright (C) 2015 Jonas Krug
 * Copyright (C) 2015 Tim Gevers
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#include "ai-core.h"
#include "ai-pathfinding.h"
#include "ai-simulation.h"
#include "gameplay-players.h"
#include "gameplay.h"
#include "core.h"

/**
 * This function updates the player actions. This is only processed for AI
 * players. It generates a job list and chooses a job by AI criteria. As final
 * step it moves the player to execute the choosed job.
 * 
 * @param player The player to process.
 */
void ai_core_update(gameplay_players_player_t *player)
{
	int x = 0;
	int y = 0;
	ai_jobs_t *job = NULL;
	gameplay_players_player_t *player_user = NULL;
	
	player_user = gameplay_players_get_user();
	if(player_user == NULL)
	{
		core_error("Failed to find user controlled player.");
		return;
	}
	
	if(player->type != GAMEPLAY_PLAYERS_TYPE_AI)
	{
		return;
	}
	
	if(player->jobs != NULL)
	{
		ai_jobs_free(&(player->jobs));
	}
	
	// all tiles are potential bomb drop spots
	for(y = 0; y < GAMEPLAY_FIELD_HEIGHT; y++)
	{
		for(x = 0; x < GAMEPLAY_FIELD_WIDTH; x++)
		{
			job = ai_jobs_allocate(x, y, BOMB_DROP);
			ai_jobs_insert(&(player->jobs), job);
		}
	}
	
	// remove tiles which are not walkable (a.k.a. non floor tiles)
	for(y = 0; y < GAMEPLAY_FIELD_HEIGHT; y++)
	{
		for(x = 0; x < GAMEPLAY_FIELD_WIDTH; x++)
		{
			if(gameplay_get_walkable(x, y, 0) == 0)
			{
				// core_debug("Remove (%i, %i), cause: walkable", x, y);
				ai_jobs_remove(&(player->jobs), x, y, BOMB_DROP);
			}
		}
	}
	
	// remove tiles which are not accessable
	for(y = 0; y < GAMEPLAY_FIELD_HEIGHT; y++)
	{
		for(x = 0; x < GAMEPLAY_FIELD_WIDTH; x++)
		{
			if(ai_pathfinding_move_to_length(player->position_x, player->position_y, x, y, 0) == -1)
			{
				// core_debug("Remove (%i, %i), cause: pathfinding", x, y);
				ai_jobs_remove(&(player->jobs), x, y, BOMB_DROP);
			}
		}
	}
	
	// remove tiles which have no save place for hiding
	for(y = 0; y < GAMEPLAY_FIELD_HEIGHT; y++)
	{
		for(x = 0; x < GAMEPLAY_FIELD_WIDTH; x++)
		{
			if(ai_simulation_validate_tile(player->explosion_radius, x, y) == 0)
			{
				// core_debug("Remove (%i, %i), cause: unsafe", x, y);
				ai_jobs_remove(&(player->jobs), x, y, BOMB_DROP);
			}
		}
	}
	
	// add escape job
	if(ai_simulation_get_walkable(player->position_x, player->position_y) == 0)
	{
		// all tiles are potential escape spots
		for(y = 0; y < GAMEPLAY_FIELD_HEIGHT; y++)
		{
			for(x = 0; x < GAMEPLAY_FIELD_WIDTH; x++)
			{
				job = ai_jobs_allocate(x, y, ESCAPE);
				ai_jobs_insert(&(player->jobs), job);
			}
		}
		
		// remove tiles which are not walkable (a.k.a. non floor tiles)
		for(y = 0; y < GAMEPLAY_FIELD_HEIGHT; y++)
		{
			for(x = 0; x < GAMEPLAY_FIELD_WIDTH; x++)
			{
				if(gameplay_get_walkable(x, y, 0) == 0)
				{
					ai_jobs_remove(&(player->jobs), x, y, ESCAPE);
				}
			}
		}
	}
	
	// remove current tile
	// ai_jobs_remove(&(player->jobs), player->position_x, player->position_y, BOMB_DROP);
	
	job = ai_jobs_get_optimal(player->jobs, player_user->position_x, player_user->position_y, player->position_x, player->position_y);
	
	// ai_jobs_print(player->jobs);
	
	if(job != NULL && player->movement_cooldown == 0)
	{
		switch(job->type)
		{
			case ESCAPE:
			{
				if(ai_pathfinding_move_to_next(player->position_x, player->position_y, job->position_x, job->position_y, &x, &y, 2) != -1)
				{
					player->position_x = x;
					player->position_y = y;
					player->movement_cooldown = player->movement_cooldown_initial;
				}
				
				break;
			}
			case BOMB_DROP:
			{
				if(ai_pathfinding_move_to_next(player->position_x, player->position_y, job->position_x, job->position_y, &x, &y, 0) != -1)
				{
					player->position_x = x;
					player->position_y = y;
					player->movement_cooldown = player->movement_cooldown_initial;
					
					if(player->position_x == job->position_x && player->position_y == job->position_y)
					{
						gameplay_players_place_bomb(player);
					}
				}
				
				break;
			}
		}
	}
}

/**
 * This function cleans the job list of a player.
 * 
 * @param player The player to be cleaned.
 */
void ai_core_cleanup(gameplay_players_player_t *player)
{
	if(player->type != GAMEPLAY_PLAYERS_TYPE_AI)
	{
		return;
	}
	
	if(player->jobs != NULL)
	{
		ai_jobs_free(&(player->jobs));
	}
}
