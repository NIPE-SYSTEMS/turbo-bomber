# Copyright (C) 2015 NIPE-SYSTEMS
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

CC = gcc

CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += `sdl2-config --cflags`
CFLAGS += `pkg-config --cflags openal`

LIBS += `sdl2-config --libs`
LIBS += -lSDL2_mixer
LIBS += `pkg-config --libs openal`

PROGRAM_NAME = project-game

SRC += main.c
SRC += core.c
SRC += sound.c
SRC += timing.c
SRC += rendering.c

OBJS = $(addprefix bin/obj/, $(SRC:%.c=%.o))

.PHONY: all $(PROGRAM_NAME) init clean

all: init $(PROGRAM_NAME)

$(PROGRAM_NAME): init $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o bin/$(PROGRAM_NAME) $(LIBS)

init:
	mkdir -p bin/obj

bin/obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

clean:
	rm -Rf bin