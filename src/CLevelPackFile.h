#ifndef CLEVELPACKFILE_H
#define CLEVELPACKFILE_H

#include "string.h"
#include "CLevelPackFile.h"
#include "Defines.h"
#include "Levels.h"

#define MAXLEVELS 355
#define MAXITEMCOUNT NrOfCols*NrOfRows

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
	int width;
	int height;
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
	int n = strlen(s);
	for (int i = 0; i < n; i++)
		s[i] = tolower(s[i]);
}

int *strchr(int *s, int c)
{
   while(*s != c && *s != 0) {
      s++;
   }
   if(*s == c) {
      return s;
   }else {
      return NULL;
   }
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
	int ilen;
	bool inlevel = false;
	LevelPackFile->LevelCount = 0;
	memset(LevelPackFile->author, 0, MAXAUTHORLEN);
	memset(LevelPackFile->set, 0, MAXSETLEN);
	while(*pchar != 0)
	{
		linepos = 0;
		while((*pchar != '\n') && (*pchar != 0))
		{
			if(*pchar != '\r')
				if(linepos < MAXLINELEN-1)
					line[linepos++] = tolower(*pchar);
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
				ilen = strlen(pset);
				if(ilen < MAXSETLEN-1)
					strncpy(LevelPackFile->set, pset, ilen);
				else
					strncpy(LevelPackFile->set, pset, MAXSETLEN-1);
			}

			pauthor = strstr(line, "author:");
			if(pauthor)
			{
				pauthor+= 7;
				while(*pauthor == ' ')
					pauthor++;
				ilen = strlen(pauthor);
				if(ilen < MAXAUTHORLEN-1)
					strncpy(LevelPackFile->author, pauthor, ilen);
				else
					strncpy(LevelPackFile->author, pauthor, MAXAUTHORLEN-1);
			}
		}

		//found double point while in a level start a metadata field
		pdoublepoint = strstr(line, ":");			
		if(inlevel && pdoublepoint)
		{
			if(strlen(levelField) > 0)
			{
				int* ptmp = levelFieldValue;
				while(*ptmp == ' ')
					ptmp++;
				StringToLower(levelField);
				if (strcmp(levelField, "title") == 0)
				{
					ilen = strlen(ptmp);
					if(ilen < MAXTITLELEN-1)
						strncpy(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].title, ptmp, ilen);
					else
						strncpy(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].title, ptmp, MAXTITLELEN-1);
				}

				if (strcmp(levelField, "author") == 0)
				{
					ilen = strlen(ptmp);
					if(ilen < MAXAUTHORLEN-1)
						strncpy(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].author, ptmp, ilen);
					else
						strncpy(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].author, ptmp, MAXAUTHORLEN-1);
				}

				if (strcmp(levelField, "comment") == 0)
				{
					ilen = strlen(ptmp);
					if(ilen < MAXCOMMENTLEN-1)
						strncpy(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].comments, ptmp, ilen);
					else
						strncpy(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].comments, ptmp, MAXCOMMENTLEN-1);
				}
			}	
			memset(levelFieldValue, 0, MAXLEVELFIELDDATALEN);
			memset(levelField, 0, MAXLEVELFIELDLEN);
			strncpy(levelField, line, pdoublepoint - &line[0]);
			strncpy(levelFieldValue, pdoublepoint +1, strlen(line));
			continue;
		}
		
		//we are in a level but found no empty line and no doublepoint then we are then in a multiline metadata field just append its value
		if(inlevel && !(*pline == 0) && !pdoublepoint && (levelField[0] != 0))
		{
			if(strlen(levelFieldValue) > 0)
				strcat(levelFieldValue, "\n");
			strcat(levelFieldValue, line);
			continue;
		}

		//we are in a level and found a empty line then assume level end
		if(inlevel && (*pline == 0))
		{
			if(strlen(levelField) > 0)
			{
				int* ptmp = levelFieldValue;
				while(*ptmp == ' ')
					ptmp++;
				StringToLower(levelField);
				if (strcmp(levelField, "title") == 0)
				{
					ilen = strlen(ptmp);
					if(ilen < MAXTITLELEN-1)
						strncpy(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].title, ptmp, ilen);
					else
						strncpy(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].title, ptmp, MAXTITLELEN-1);
				}

				if (strcmp(levelField, "author") == 0)
				{
					ilen = strlen(ptmp);
					if(ilen < MAXAUTHORLEN-1)
						strncpy(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].author, ptmp, ilen);
					else
						strncpy(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].author, ptmp, MAXAUTHORLEN-1);
				}

				if (strcmp(levelField, "comment") == 0)
				{
					ilen = strlen(ptmp);
					if(ilen < MAXCOMMENTLEN-1)
						strncpy(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].comments, ptmp, ilen);
					else
						strncpy(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].comments, ptmp, MAXCOMMENTLEN-1);
				}
			}
			//clear them for if condition above conerning level start
			memset(levelFieldValue, 0, MAXLEVELFIELDDATALEN);
			memset(levelField, 0, MAXLEVELFIELDLEN);
			inlevel = false;
			if((LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].width <= maxWidth) && 
				(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].height <= maxHeight))
				LevelPackFile->LevelCount++;
			continue;
		}

		//we are not in a level and found a wall and no doublepoint and we are not in a levelfield then assume levelstart
		if (!inlevel && (strchr(line, LPWall)) && !pdoublepoint && (levelField[0] == 0))
		{
			if (MetaOnly)
				return true;
			inlevel=true;
			y = 0;
			LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].width = 0;
			LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].height = 0;
			LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].minx = NrOfCols;
			LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].miny = NrOfRows;
			LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].maxx = 0;
			LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].maxy = 0;
			memset(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].author, 0, MAXAUTHORLEN);
			memset(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].title, 0, MAXTITLELEN);
			memset(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].comments, 0, MAXCOMMENTLEN);
			LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts = 0;
		}

		//we are in level and not in a level meta field
		if(inlevel && (levelField[0] == 0))
		{
			int linelen = strlen(line);
			for(int x = 0; x < linelen; x++)
			{
				//DON'T EXCEED MAX ITEMCOUNT!
				if(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts >= MAXITEMCOUNT)
				{
					LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].width = 1000;
					break;
				}

				switch(line[x])
				{
					case LPBox:
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].x = x;
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].y = y;
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].id = IDBox;
						LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts++;
						break;
					case LPPlayer:
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].x = x;
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].y = y;
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].id = IDPlayer;
						LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts++;
						break;
					case LPBoxOnSpot:
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].x = x;
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].y = y;
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].id = IDSpot;
						LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts++;
						//DON'T EXCEED MAX ITEMCOUNT!
						if(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts >= MAXITEMCOUNT)
						{
							LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].width = 1000;
							break;
						}
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].x = x;
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].y = y;
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].id = IDBox;
						LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts++;
						break;
					case LPSpot:
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].x = x;
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].y = y;
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].id = IDSpot;
						LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts++;
						break;
					case LPPlayerOnSpot:
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].x = x;
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].y = y;
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].id = IDSpot;
						LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts++;
						//DON'T EXCEED MAX ITEMCOUNT!
						if(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts >= MAXITEMCOUNT)
						{
							LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].width = 1000;
							break;
						}
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].x = x;
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].y = y;
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].id = IDPlayer;
						LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts++;
						break;
					case LPWall:						
						if(x < LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].minx)
							LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].minx = x;
						if(x > LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].maxx)
							LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].maxx = x;						
						if(y < LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].miny)
							LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].miny = y;
						if(y > LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].maxy)
							LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].maxy = y;

						if(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].width < x+1)
							LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].width = x+1;
						if(LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].height < y+1)
							LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].height = y+1;
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].x = x;
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].y = y;
						LevelPackFile->Levels[LevelPackFile->LevelCount][LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts].id = IDWall;
						LevelPackFile->LevelsMeta[LevelPackFile->LevelCount].parts++;
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
	bool Result = false;
	LevelPackFile->LevelCount = 0;
	LevelPackFile->Loaded = false;
	memset(LevelPackFile->author, 0, MAXAUTHORLEN);
	memset(LevelPackFile->set, 0, MAXSETLEN);
	Result = CLevelPackFile_parseText(LevelPackFile, LevelPacks[Level], maxWidth, maxHeight, MetaOnly);		
	LevelPackFile->Loaded = true;
	return Result;
}

#endif