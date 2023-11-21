/**********************************************************************
 * Copyright (c) 2021-2022
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned char bool;
#define true	1
#define false	0

#include "list_head.h"
struct threadArg{
	struct socketInfo* p1;
	struct socketInfo* p2;
};
struct score{
        int one;
        int two;
        int three;
        int four;
        int five;
        int six;
        int three_of_a_kind;
        int four_of_a_kind;
        int full_house;
        int small_straight;
        int large_straight;
        int chance;
        int yahtzee;
	int bonus;
        int sum;
};
struct uinfo{
        char name[16];
        int win;
        int draw;
        int lose;
};

struct socketInfo{
    struct list_head list;
    int fd;
    int id;
    struct uinfo infos;
    struct socketInfo* vs;
    struct score result;
    int scores[15];
    int dices[5];
    int step;
};
struct turn{
	pthread_mutex_t turn;
	int id;
};
struct entry {
	struct list_head list;
	char *string;
};

#endif
