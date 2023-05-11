// Fill out your copyright notice in the Description page of Project Settings.


#include "SingletonData.h"

USingletonData* USingletonData::GetInstance()
{
    if (instance == NULL)
    {
        instance = NewObject<USingletonData>();
    }
    return instance;
}

void USingletonData::SetData(int playercount, int serverport, FString ip)
{
    data.PlayerCount = playercount;
    data.ServerPort = serverport;
    data.IP = ip;

    UE_LOG(LogTemp, Error, TEXT("싱글톤 Port : %d"), data.ServerPort);
    UE_LOG(LogTemp, Error, TEXT("싱글톤 IP : %s"), *data.IP);
}
