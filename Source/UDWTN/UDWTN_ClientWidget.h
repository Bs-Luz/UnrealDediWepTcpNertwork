// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableText.h"
#include "Components/Button.h"
#include "UObject/ConstructorHelpers.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "IPAddress.h"
#include "Json.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "UDWTN_ClientWidget.generated.h"

/**
 * 
 */
UCLASS()
class UDWTN_API UUDWTN_ClientWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	UEditableText* Id;

	UPROPERTY(BlueprintReadWrite)
	UEditableText* Password;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString FId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString FPassword;
		
	void NativeConstruct();
	UFUNCTION(BlueprintCallable)
	void OnButtonClickToLogin();
	UFUNCTION(BlueprintCallable)
	void OnButtonClickToSignUp();
	UFUNCTION(BlueprintCallable)
	void OnButtonClickToCreateSignUp();
	UFUNCTION(BlueprintCallable)
	void OnButtonClickToBack();

	bool ConnectToDedicatedServer();
	void HttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);

private:
	FString TcpSererIpAdd = TEXT("192.168.0.53");
	int32 TcpServerPort = 10101;
	FString ClientInfo;
	FSocket* Socket;
};