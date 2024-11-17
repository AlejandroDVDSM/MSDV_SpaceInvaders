// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InvaderSquad.generated.h"

enum class InvaderMovementType : uint8;

UCLASS()
class SPACEINVADERS_API AInvaderSquad : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY()
	class USceneComponent* Root;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Squad movement")
	InvaderMovementType state;

	UPROPERTY(VisibleAnyWhere, BlueprintReadWrite, Category = "Squad movement")
	InvaderMovementType previousState;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Squad movement")
	float freeJumpRate;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Squad movement")
	float horizontalVelocity;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Squad movement")
	float verticalVelocity;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Squad movement")
	float velocityIncreaser;

	AInvaderSquad();

	UFUNCTION(BlueprintCallable)
	void UpdateSquadState(float delta);

	UFUNCTION(BlueprintCallable)
	float GetHorizontalVelocity();

	UFUNCTION(BlueprintCallable)
	float GetVerticalVelocity();

	UFUNCTION(BlueprintCallable)
	void IncrementVelocitySquad();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Squad Spawner")
	TSubclassOf<class AInvader> invaderClass;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Squad Spawner")
	int32 nRows;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Squad Spawner")
	int32 nCols;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Squad Spawner")
	float extraSeparation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<class AInvader*> SquadMembers;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

private:
	int32 numberOfMembers;

	UPROPERTY()
	class AInvader* invaderTemplate;

	float timeFromLastFreeJump;

	void SquadOnLeftSide();

	void SquadOnRightSide();

	void SquadFinishesDown();

	void RemoveInvader(int32 ind);

	UPROPERTY()
	class ASIGameModeBase* MyGameMode;

	// Values for initializing defaults
	static const int32 defaultNRows = 1;
	static const int32 defaultNCols = 1;
	static constexpr const float defaultHorizontalVelocity = 1000.0f;
	static constexpr const float defaultVerticalVelocity = 1000.0f;
	static constexpr const float defaultExtraSeparation = 0.0f;
};
