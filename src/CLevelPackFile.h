#ifndef CLEVELPACKFILE_H
#define CLEVELPACKFILE_H

#include "string.h"
#include "CLevelPackFile.h"
#include "Defines.h"
#include "Levels.h"
#include "Debugspeed.h"

#define MAXLEVELS 355
#define MAXITEMCOUNT (NrOfCols*NrOfRows) + 2

#define MAXCOMMENTLEN 150
#define MAXSETLEN 50
#define MAXAUTHORLEN 75
#define MAXTITLELEN 50
#define MAXLEVELFIELDDATALEN 2000
#define MAXLEVELFIELDLEN 100
#define MAXLINELEN 2000

#define LPWall '#'
#define LPSpot '.'
#define LPPlayer '@'
#define LPBox '$'
#define LPPlayerOnSpot '+'
#define LPBoxOnSpot '*'
#define LPFloor 'A'


struct LevelPart
{
	int id;
	int x;
	int y;
};

struct LevelMeta
{
	int minx;
	int miny;
	int maxx;
	int maxy;
	int parts;
	int[MAXAUTHORLEN] author;
	int[MAXCOMMENTLEN] comments;
	int[MAXTITLELEN] title;

};

struct CLevelPackFile
{
	LevelPart[MAXLEVELS][MAXITEMCOUNT] Levels;
	LevelMeta[MAXLEVELS] LevelsMeta;
	int[MAXAUTHORLEN] author;
	int[MAXSETLEN] set;
	int LevelCount;
	bool Loaded;
};


CLevelPackFile* CLevelPackFile_Create()
{
	CLevelPackFile* Result = (CLevelPackFile*)malloc(sizeof(CLevelPackFile));
	Result->Loaded = false;
	Result->LevelCount = 0;
	memset(Result->author, 0, MAXAUTHORLEN);
	memset(Result->set, 0, MAXSETLEN);
	return Result;
}

void CLevelPackFile_Destroy(CLevelPackFile* LevelPackFile)
{
	if(LevelPackFile)
	{
		free(LevelPackFile);
		LevelPackFile = NULL;
	}
}

int *strstr(int *s1, int *s2)
{
    int n = strlen(s2);
    while(*s1)
        if(!memcmp(s1++,s2,n))
            return (int *) (s1-1);
    return NULL;
}

void StringToLower(int *s)
{
	while(*s)
	{
		*s = tolower(*s);
		s++;
	}
}

int *strchr(int *s, int c)
{
	int n = 1;
    while(*s)
        if(!memcmp(s++,&c,n))
            return (int *) (s-1);
    return NULL;
}

bool CLevelPackFile_parseText(CLevelPackFile *LevelPackFile, int* text, int maxWidth, int maxHeight, bool MetaOnly)
{
	int[MAXLINELEN] line = "";
	int[MAXLEVELFIELDLEN] levelField = "";
	int[MAXLEVELFIELDDATALEN] levelFieldValue = "";
	int linepos;
	int* pchar = text;
	int* pdoublepoint, pset, pauthor;
	int y;
	bool inlevel = false;
	LevelPackFile->LevelCount = 0;
	memset(LevelPackFile->author, 0, MAXAUTHORLEN);
	memset(LevelPackFile->set, 0, MAXSETLEN);
	LevelMeta* levelMeta = &LevelPackFile->LevelsMeta[LevelPackFile->LevelCount];
	while(*pchar != 0)
	{
		linepos = 0;
		while((*pchar != '\n') && (*pchar != 0))
		{
			if((*pchar != '\r') && (linepos < MAXLINELEN-1))
			{
				line[linepos++] = tolower(*pchar);
			}
			pchar++;
		}

		if(*pchar == 0)
			break;

		pchar++;

		line[linepos] = 0;
		int* pline = line;

		if(LevelPackFile->LevelCount == 0)
		{
			pset = strstr(line, "set:");
			if(pset)
			{
				pset+= 4;
				while(*pset == ' ')
					pset++;
				strncpy(LevelPackFile->set, pset, MAXSETLEN-1);
			}

			pauthor = strstr(line, "author:");
			if(pauthor)
			{
				pauthor+= 7;
				while(*pauthor == ' ')
					pauthor++;
				strncpy(LevelPackFile->author, pauthor, MAXAUTHORLEN-1);
			}
		}

		//found double point while in a level start a metadata field
		pdoublepoint = strstr(line, ":");
		if(inlevel && pdoublepoint)
		{
			if(levelField[0] != 0)
			{
				int* ptmp = levelFieldValue;
				while(*ptmp == ' ')
					ptmp++;
				StringToLower(levelField);
				if (strcmp(levelField, "title") == 0)
				{
					strncpy(levelMeta->title, ptmp, MAXTITLELEN-1);
				}

				if (strcmp(levelField, "author") == 0)
				{
					strncpy(levelMeta->author, ptmp, MAXAUTHORLEN-1);
				}

				if (strcmp(levelField, "comment") == 0)
				{
					strncpy(levelMeta->comments, ptmp, MAXCOMMENTLEN-1);
				}
			}	
			memset(levelFieldValue, 0, MAXLEVELFIELDDATALEN);
			memset(levelField, 0, MAXLEVELFIELDLEN);
			strncpy(levelField, line, pdoublepoint - &line[0]);
			strncpy(levelFieldValue, pdoublepoint +1, MAXLEVELFIELDDATALEN - 1);
			continue;
		}
		
		//we are in a level but found no empty line and no doublepoint then we are then in a multiline metadata field just append its value
		if(inlevel && !(*pline == 0) && !pdoublepoint && (levelField[0] != 0))
		{
			if(levelFieldValue[0] != 0)
				strcat(levelFieldValue, "\n");
			strcat(levelFieldValue, line);
			continue;
		}

		//we are in a level and found a empty line then assume level end
		if(inlevel && (*pline == 0))
		{
			if(levelField[0] != 0)
			{
				int* ptmp = levelFieldValue;
				while(*ptmp == ' ')
					ptmp++;
				StringToLower(levelField);
				if (strcmp(levelField, "title") == 0)
				{
					strncpy(levelMeta->title, ptmp, MAXTITLELEN-1);
				}

				if (strcmp(levelField, "author") == 0)
				{
					strncpy(levelMeta->author, ptmp, MAXAUTHORLEN-1);
				}

				if (strcmp(levelField, "comment") == 0)
				{
					strncpy(levelMeta->comments, ptmp, MAXCOMMENTLEN-1);
				}
			}
			//clear them for if condition above conerning level start
			memset(levelFieldValue, 0, MAXLEVELFIELDDATALEN);
			memset(levelField, 0, MAXLEVELFIELDLEN);
			inlevel = false;
			if((levelMeta->maxx+1 <= maxWidth) && 
				(levelMeta->maxy+1 <= maxHeight))
			{
				LevelPackFile->LevelCount++;
				levelMeta = &LevelPackFile->LevelsMeta[LevelPackFile->LevelCount];
			}
			continue;
		}

		//we are not in a level and found a wall and no doublepoint and we are not in a levelfield then assume levelstart
		if (!inlevel && !pdoublepoint && (levelField[0] == 0) && (strchr(line, LPWall)))
		{
			if (MetaOnly)
				return true;
			inlevel=true;
			y = 0;
			levelMeta->minx = NrOfCols;
			levelMeta->miny = NrOfRows;
			levelMeta->maxx = 0;
			levelMeta->maxy = 0;
			memset(levelMeta->author, 0, MAXAUTHORLEN);
			memset(levelMeta->title, 0, MAXTITLELEN);
			memset(levelMeta->comments, 0, MAXCOMMENTLEN);
			levelMeta->parts = 0;
		}

		//we are in level and not in a level meta field
		if(inlevel && (levelField[0] == 0))
		{			
			for(int x = 0; x < linepos; x++)
			{
				//there should be always still a wall so it's safe
				if (line[x] == ' ')
					continue;
				

				LevelPart* levelPart = &LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts];
				
				//DON'T EXCEED MAX ITEMCOUNT!
				if(levelMeta->parts+2 >= MAXITEMCOUNT)
				{
					levelMeta->maxx = 1000;
					break;
				}
				
				switch(line[x])
				{
					case LPWall:
						if(x < levelMeta->minx)
							levelMeta->minx = x;
						if(x > levelMeta->maxx)
							levelMeta->maxx = x;
						if(y < levelMeta->miny)
							levelMeta->miny = y;
						if(y > levelMeta->maxy)
							levelMeta->maxy = y;
						levelPart->x = x;
						levelPart->y = y;
						levelPart->id = IDWall;
						levelMeta->parts++;
						break;
					case LPBox:
						levelPart->x = x;
						levelPart->y = y;
						levelPart->id = IDBox;
						levelMeta->parts++;
						break;
					case LPBoxOnSpot:
						levelPart->x = x;
						levelPart->y = y;
						levelPart->id = IDSpot;
						levelMeta->parts++;
						
						levelPart = &LevelPackFile->Levels[LevelPackFile->LevelCount][levelMeta->parts];
						
						levelPart->x = x;
						levelPart->y = y;
						levelPart->id = IDBox;
						levelMeta->parts++;
						break;
					case LPSpot:
						levelPart->x = x;
						levelPart->y = y;
						levelPart->id = IDSpot;
						levelMeta->parts++;
						break;
					case LPPlayerOnSpot:
						levelPart->x = x;
						levelPart->y = y;
						levelPart->id = IDSpot;

						levelPart = &LevelPackFile->Levels[LevelPackFile->LevelCount][levelMeta->parts];
						
						levelPart->x = x;
						levelPart->y = y;
						levelPart->id = IDPlayer;
						levelMeta->parts++;
						break;
					case LPPlayer:
						levelPart->x = x;
						levelPart->y = y;
						levelPart->id = IDPlayer;
						levelMeta->parts++;
						break;
				}
			}
			y++;
		}
	}
	return LevelPackFile->LevelCount > 0;
}


bool CLevelPackFile_loadFile(CLevelPackFile* LevelPackFile, int Level , int maxWidth, int maxHeight, bool MetaOnly)
{
#ifdef DEBUG
	StartDebugSpeed(5);
#endif
	bool Result = false;
	LevelPackFile->LevelCount = 0;
	LevelPackFile->Loaded = false;
	memset(LevelPackFile->author, 0, MAXAUTHORLEN);
	memset(LevelPackFile->set, 0, MAXSETLEN);
	Result = CLevelPackFile_parseText(LevelPackFile, LevelPacks[Level], maxWidth, maxHeight, MetaOnly);		
	LevelPackFile->Loaded = true;
#ifdef DEBUG
	StopDebugSpeed(5);
#endif
	return Result;
}

#endif