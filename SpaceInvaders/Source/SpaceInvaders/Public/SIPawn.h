// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "SIPawn.generated.h"

UCLASS()
class SPACEINVADERS_API ASIPawn : public ADefaultPawn
{
	GENERATED_BODY()

public:
	/** MappingContext for player input. **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	UInputMappingContext* InputMapping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* inputToMove;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* inputToFire;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* inputToPause;

	//Points per invader
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defender config")
	int32 pointsPerInvader;

	//Points per squad
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defender config")
	int32 pointsPerSquad;

	//Lifes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defender config")
	int32 playerLifes;
	
	// Velocity of the pawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Defender config")
	float velocity;
	
	// Velocity of the player bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defender config")
	float bulletVelocity;

	// Bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defender config")
	TSubclassOf<class ABullet> bulletClass;

	//Audios 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defender config")
	class USoundCue* AudioShoot; // SoundCue para albergar el sonido del disparo.

	//Audios: Explossion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defender config")
	class USoundCue* AudioExplosion;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Defender config")
	class UNiagaraSystem* ExplosionEffect;
	
	// Sets default values for this pawn's properties
	ASIPawn();

	// It could be possible to change the static mesh component during run time.
	UFUNCTION(BlueprintCallable)
	void SetStaticMesh(class UStaticMesh* staticMesh = nullptr, FString path = TEXT(""), FVector scale = FVector(1.0f, 1.0f, 1.0f));

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Overlaps
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	//Getters and Setters

	UFUNCTION(BlueprintCallable)
	int64 GetPoints();
	
	UFUNCTION(BlueprintCallable)
	int32 GetLifes();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Bindings for inputs
	void OnEnhancedMove(const FInputActionValue& Value);

	void OnEnhancedFire();

	void OnEnhancedPause();
	// void OnMove(float value);

	// void OnFire();

	// void OnPause();

	UFUNCTION(BlueprintCallable)
	void DestroyPlayer();

	UFUNCTION(BlueprintCallable)
	void PostPlayerDestroyed();

private:
	// To set a frozen state (no moving and firing capabilities)
	bool bFrozen;

	//To pause the Game
	bool bPause;

	FTimerHandle timerHandle;

	UPROPERTY()
	class ASIGameModeBase* MyGameMode;
	
	//Points
	int64 playerPoints;

	
	
	UPROPERTY()
	class ABullet* bulletTemplate; // Instancia de una bala que se usa como "molde" para un spawning eficiente.

	UPROPERTY()
	class UAudioComponent* AudioComponent; // Reproductor de audio del Pawn.

	// Bindings to delegates
	void InvaderDestroyed(int32 id);
	void SquadDissolved(int32 val);
	void SquadSuccessful();

	static constexpr const TCHAR* defaultStaticMeshPath = TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'");
};
