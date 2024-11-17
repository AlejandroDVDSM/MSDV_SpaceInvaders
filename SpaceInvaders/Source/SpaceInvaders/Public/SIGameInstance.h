// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SIGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SPACEINVADERS_API USIGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	int64 GetRecord();

	void UpdateRecord(int64 newRecord);

private:
	int64 playerRecord;
};
