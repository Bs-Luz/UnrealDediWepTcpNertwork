// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonData.generated.h"

/**
 * 
 */
UCLASS()
class UDWTN_API USingletonData : public UObject
{
	GENERATED_BODY()
	
public:
	static USingletonData* GetInstance();

	struct Data
	{
		int PlayerCount;
		int ServerPort;
		FString IP;
	};

	const Data& GetData() { return data; }
	void SetData(int playernum, int serverport, FString ip);

	bool GetBool() const { return singletonbool; }
	void SerBool(bool val) { singletonbool = val; }

private:
	USingletonData()
	{
		data.PlayerCount = 0;
	}
	static USingletonData* instance;
	static TSubclassOf<USingletonData> SingletonClass;

	Data data;
	bool singletonbool;
};
USingletonData* USingletonData::instance = NULL;	//싱글톤 초기화