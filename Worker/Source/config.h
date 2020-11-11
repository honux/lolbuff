#ifndef __CONFIG_H__
#define __CONFIG_H__

struct config
{
    char serverAddr[256];
    char serverPort[256];

    char leagueVersion[256];
    char leagueLoginServerAddress[256];
    char leaguegameServerAddress[256];
    char leaguegameServerPort[256];
};

#endif