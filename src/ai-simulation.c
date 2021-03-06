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

#include <stdio.h>
#include <stdlib.h>

#include "ai-simulation.h"
#include "ai-pathfinding.h"
#include "gameplay.h"
#include "core.h"

static void ai_simulation_reset_simulated(void);

/**
 * This function resets all normal simulation flags of the field.
 */
void ai_simulation_reset(void)
{
	int x = 0;
	int y = 0;
	gameplay_field_t *field = NULL;
	
	field = gameplay_get_field();
	
	if(field == NULL)
	{
		return;
	}
	
	for(y = 0; y < GAMEPLAY_FIELD_HEIGHT; y++)
	{
		for(x = 0; x < GAMEPLAY_FIELD_WIDTH; x++)
		{
			GAMEPLAY_FIELD(field, x, y).ai_simulation_walkable = 1;
		}
	}
}

static void ai_simulation_reset_simulated(void)
/**
 * This function resets all special simulation flags of the field.
 */
{
	int x = 0;
	int y = 0;
	gameplay_field_t *field = NULL;
	
	field = gameplay_get_field();
	
	if(field == NULL)
	{
		return;
	}
	
	for(y = 0; y < GAMEPLAY_FIELD_HEIGHT; y++)
	{
		for(x = 0; x < GAMEPLAY_FIELD_WIDTH; x++)
		{
			GAMEPLAY_FIELD(field, x, y).ai_simulation_walkable_simulated = 1;
		}
	}
}

/**
 * This function acts as a helper function to store a simulation flag in the
 * field.
 * 
 * @param position_x The x coordinate of the tile.
 * @param position_y The y coordinate of the tile.
 * @param simulated The simulation flag which should be set. 0 means normal bomb
 *                  on the field (placed by other players), 1 means virtual bomb
 *                  (placed by the simulation algorithm).
 */
void ai_simulation_explosion_set_unwalkable(int position_x, int position_y, char simulated)
{
	gameplay_field_t *field = NULL;
	
	field = gameplay_get_field();
	
	if(field == NULL)
	{
		return;
	}
	
	if(simulated == 0) // bombs on the field
	{
		GAMEPLAY_FIELD(field, position_x, position_y).ai_simulation_walkable = 0;
	}
	else // virtual bombs
	{
		GAMEPLAY_FIELD(field, position_x, position_y).ai_simulation_walkable_simulated = 0;
	}
}

/**
 * This function simulates an explosion. It can simulate real bombs on the field
 * or virtual bombs. After the execution of the function it is possible to test
 * where the explosions will be. This can be called multiple times.
 * 
 * @param position_x The x coordinate of the simulated bomb.
 * @param position_y The y coordinate of the simulated bomb.
 * @param simulated The simulation flag which should be set. 0 means normal bomb
 *                  on the field (placed by other players), 1 means virtual bomb
 *                  (placed by the simulation algorithm).
 */
void ai_simulation_explosion(int position_x, int position_y, int explosion_radius, char simulated)
{
	int x = 0;
	int y = 0;
	
	for(x = position_x; x < GAMEPLAY_FIELD_WIDTH && x < position_x + explosion_radius && gameplay_get_walkable(x, position_y, 1); x++)
	{
		ai_simulation_explosion_set_unwalkable(x, position_y, simulated);
	}
	
	for(x = position_x - 1; x > 0 && x > position_x - explosion_radius && gameplay_get_walkable(x, position_y, 1); x--)
	{
		ai_simulation_explosion_set_unwalkable(x, position_y, simulated);
	}
	
	for(y = position_y; y < GAMEPLAY_FIELD_HEIGHT && y < position_y + explosion_radius && gameplay_get_walkable(position_x, y, 1); y++)
	{
		ai_simulation_explosion_set_unwalkable(position_x, y, simulated);
	}
	
	for(y = position_y - 1; y > 0 && y > position_y - explosion_radius && gameplay_get_walkable(position_x, y, 1); y--)
	{
		ai_simulation_explosion_set_unwalkable(position_x, y, simulated);
	}
}

/**
 * This function copies all fire flags from the field to the simulation field.
 */
void ai_simulation_copy_fire(void)
{
	int x = 0;
	int y = 0;
	gameplay_field_t *field = NULL;
	
	field = gameplay_get_field();
	
	if(field == NULL)
	{
		return;
	}
	
	for(y = 0; y < GAMEPLAY_FIELD_HEIGHT; y++)
	{
		for(x = 0; x < GAMEPLAY_FIELD_WIDTH; x++)
		{
			if(GAMEPLAY_FIELD(field, x, y).fire == 1)
			{
				ai_simulation_explosion_set_unwalkable(x, y, 0);
			}
		}
	}
}

/**
 * This function validates if a tile is valid. On a valid tile may be placed a
 * bomb by the AI. The validation tests if there are spots to hide from the
 * explosion. It returns the amount of possible hiding places.
 * 
 * @param explosion_radius The explosion radius of the simulated bomb.
 * @param position_x The x coordinate of the simulated bomb.
 * @param position_y The y coordinate of the simulated bomb.
 * @return The amount of possible hiding places. 0 on error.
 */
int ai_simulation_validate_tile(int explosion_radius, int position_x, int position_y)
{
	int x = 0;
	int y = 0;
	int count_hiding_places = 0;
	gameplay_field_t *field = NULL;
	
	field = gameplay_get_field();
	
	if(field == NULL)
	{
		return 0;
	}
	
	ai_simulation_reset_simulated();
	ai_simulation_explosion(position_x, position_y, explosion_radius, 1);
	
	for(y = 0; y < GAMEPLAY_FIELD_HEIGHT; y++)
	{
		for(x = 0; x < GAMEPLAY_FIELD_WIDTH; x++)
		{
			if(gameplay_get_walkable(x, y, 0) == 1 && GAMEPLAY_FIELD(field, x, y).ai_simulation_walkable == 1 && GAMEPLAY_FIELD(field, x, y).ai_simulation_walkable_simulated == 1 && ai_pathfinding_move_to_length(position_x, position_y, x, y, 1) != -1)
			{
				count_hiding_places++;
			}
		}
	}
	
	return count_hiding_places;
}

/**
 * This function returns if a simulated tile is walkable.
 * 
 * @param position_x The x coordinate of the tile.
 * @param position_y The y coordinate of the tile.
 * @return The simulation walkable flag. 0 on error.
 */
int ai_simulation_get_walkable(int position_x, int position_y)
{
	gameplay_field_t *field = NULL;
	
	field = gameplay_get_field();
	
	if(field == NULL)
	{
		return 0;
	}
	
	return GAMEPLAY_FIELD(field, position_x, position_y).ai_simulation_walkable;
}
