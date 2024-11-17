// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SIGameModeBase.generated.h"


DECLARE_DELEGATE(FStandardDelegateSignature)
DECLARE_MULTICAST_DELEGATE_OneParam(FOneParamMulticastDelegateSignature, int32);
DECLARE_DELEGATE_OneParam(FOneParamDelegateSignature, int32)

UCLASS()
class SPACEINVADERS_API ASIGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	//------------------------------------------------
	// Spawned squad
	//------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level Layout")
	TSubclassOf<class AInvaderSquad> InvaderSquadClass;

	//------------------------------------------------
	// Point where the squad is spawned at
	//------------------------------------------------
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Level Layout")
	FVector spawnLocation;

	FStandardDelegateSignature SquadOnLeftSide; // Invader-> Squad 
	FStandardDelegateSignature SquadOnRightSide; // Invader -> Squad
	FStandardDelegateSignature SquadFinishesDown; // Invader -> Squad
	FStandardDelegateSignature SquadSuccessful; // Invader -> GameMode
	FOneParamMulticastDelegateSignature InvaderDestroyed; // Invader -> Squad Invader->Player

	FOneParamMulticastDelegateSignature NewSquad; // Squad -> Game Mode
	FStandardDelegateSignature PlayerZeroLifes; // Player -> Game Mode

	ASIGameModeBase();

	UFUNCTION(BlueprintCallable)
	void RegenerateSquad();

protected:
	virtual void BeginPlay() override;

	// Delegate bindings
	UFUNCTION(BlueprintCallable)
	void OnNewSquad(int32 lifes);

	void EndGame();

	UFUNCTION(BlueprintCallable)
	void OnPlayerZeroLifes();

private:
	UPROPERTY(VisibleAnywhere)
	AInvaderSquad* spawnedInvaderSquad;
};
