// Fill out your copyright notice in the Description page of Project Settings.


#include "InvaderSquad.h"
#include "InvaderMovementComponent.h"
#include "Invader.h"
#include "SIGameModeBase.h"

#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
AInvaderSquad::AInvaderSquad()
	: state{InvaderMovementType::STOP}
	  , previousState{InvaderMovementType::STOP}
	  , freeJumpRate{0.0001}
	  , horizontalVelocity{300.0}
	  , verticalVelocity{300.0}
	  , nRows{AInvaderSquad::defaultNRows}
	  , nCols{AInvaderSquad::defaultNCols}
	  , extraSeparation(AInvaderSquad::defaultExtraSeparation)
	  , numberOfMembers{nRows * nCols}
{
	PrimaryActorTick.bCanEverTick = true;

	// Create Components in actor

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = Root; // We need a RootComponent to have a base transform
}

// Called when the game starts or when spawned
void AInvaderSquad::BeginPlay()
{
	Super::BeginPlay();

	UWorld* TheWorld = GetWorld();

	// Bind to delegates
	if (TheWorld != nullptr) {
		AGameModeBase* GameMode = UGameplayStatics::GetGameMode(TheWorld);
		MyGameMode = Cast<ASIGameModeBase>(GameMode);
		if (MyGameMode != nullptr) {
			MyGameMode->SquadOnRightSide.BindUObject(this, &AInvaderSquad::SquadOnRightSide);
			MyGameMode->SquadOnLeftSide.BindUObject(this, &AInvaderSquad::SquadOnLeftSide);
			MyGameMode->SquadFinishesDown.BindUObject(this, &AInvaderSquad::SquadFinishesDown);
			MyGameMode->InvaderDestroyed.AddUObject(this, &AInvaderSquad::RemoveInvader);
		}
	}
	
	// Set Invader Template with Default Value for invaderClass
	if (invaderClass->IsChildOf<AInvader>())
		invaderTemplate = NewObject<AInvader>(this, invaderClass->GetFName(), RF_NoFlags,
		                                      invaderClass.GetDefaultObject());
	else
		invaderTemplate = NewObject<AInvader>();

	//Spawn Invaders

	FVector actorLocation = GetActorLocation();
	FVector spawnLocation = actorLocation;
	FRotator spawnRotation = FRotator(0.0f, 180.0f, 0.0f);
	// Invader Forward is oposite to Player Forward (Yaw rotation)
	FActorSpawnParameters spawnParameters;
	int32 count = 0;
	AInvader* spawnedInvader;
	float radiusX = 0.0f;
	float radiusY = 0.0f;
	for (int i = 0; i < this->nCols; i++)
	{
		for (int j = 0; j < this->nRows; j++)
		{
			//invaderTemplate->SetPositionInSquad(count);

			spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			spawnParameters.Template = invaderTemplate;
			spawnedInvader = GetWorld()->SpawnActor<AInvader>(spawnLocation, spawnRotation, spawnParameters);
			spawnedInvader->SetPositionInSquad(count);
			++count;
			SquadMembers.Add(spawnedInvader);
			float r = spawnedInvader->GetBoundRadius();
			if (r > radiusX)
				radiusX = r;
			if (r > radiusY)
				radiusY = r;
			spawnLocation.X += radiusX * 2 + this->extraSeparation;
		}
		spawnLocation.X = actorLocation.X;
		spawnLocation.Y += radiusY * 2 + this->extraSeparation;
	}

	this->numberOfMembers = count;
	this->state = InvaderMovementType::RIGHT; // Start with Right phase
}

void AInvaderSquad::UpdateSquadState(float delta)
{
	TArray<AInvader*> survivors;

	for (auto invader : SquadMembers)
	{
		//------------------------------------
		if (invader)
		{
			// very important, first nullptr is checked!.

			// First, we get de movement component
			UInvaderMovementComponent* imc = (UInvaderMovementComponent*)invader->GetComponentByClass(
				UInvaderMovementComponent::StaticClass());

			// Now, its state is updated
			if (imc)
			{
				if (imc->state != InvaderMovementType::FREEJUMP)
				{
					survivors.Emplace(invader);
					imc->horizontalVelocity = horizontalVelocity;
					imc->verticalVelocity = verticalVelocity;
					imc->state = state;
				}
			}
		}

		//------------------------------------
	}
	this->timeFromLastFreeJump += delta;
	float val = FMath::RandRange(0.0f, 1.0f);
	int32 countSurvivors = survivors.Num();
	if (countSurvivors > 0 && val < (1.0 - FMath::Exp(-freeJumpRate * this->timeFromLastFreeJump)))
	{
		int32 ind = FMath::RandRange(0, countSurvivors - 1); // Randomly select one of the living invaders
		UInvaderMovementComponent* imc = (UInvaderMovementComponent*)survivors[ind]->GetComponentByClass(
			UInvaderMovementComponent::StaticClass());
		if (imc)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::Printf(TEXT("%s on FreeJump"), *(imc->GetName())));
			survivors[ind]->fireRate *= 100;
			imc->state = InvaderMovementType::FREEJUMP;
		}
	}
}

float AInvaderSquad::GetHorizontalVelocity()
{
	return horizontalVelocity;
}

float AInvaderSquad::GetVerticalVelocity()
{
	return verticalVelocity;
}

void AInvaderSquad::IncrementVelocitySquad()
{
	horizontalVelocity += velocityIncreaser;
	verticalVelocity += velocityIncreaser;
}

// Called every frame
void AInvaderSquad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateSquadState(DeltaTime);
}

void AInvaderSquad::Destroyed()
{
	for (AInvader* invader : SquadMembers)
	{
		if (invader != nullptr)
			invader->Destroy();
	}
	Super::Destroyed();
}

void AInvaderSquad::SquadOnRightSide()
{
	previousState = InvaderMovementType::RIGHT;
	state = InvaderMovementType::DOWN;
}

// La escuadra llega al lado izquierdo

void AInvaderSquad::SquadOnLeftSide()
{
	previousState = InvaderMovementType::LEFT;
	state = InvaderMovementType::DOWN;
}

// Cada vez que un invasor completa el movimiento de descenso

void AInvaderSquad::SquadFinishesDown()
{
	static int32 countActions = 0;
	++countActions;
	if (countActions >= numberOfMembers)
	{
		countActions = 0;
		switch (previousState)
		{
		case InvaderMovementType::RIGHT:
			state = InvaderMovementType::LEFT;
			break;
		case InvaderMovementType::LEFT:
			state = InvaderMovementType::RIGHT;
			break;
		default:
			state = InvaderMovementType::STOP;
		}
	}
}


void AInvaderSquad::RemoveInvader(int32 ind)
{
	SquadMembers[ind] = nullptr;
	--this->numberOfMembers;
	if (this->numberOfMembers == 0)
	{
		if (MyGameMode != nullptr)
		{
			MyGameMode->NewSquad.Broadcast(1); // parameter larger than 0 to avoid finishing game!
		}
	} /*else
	{
		horizontalVelocity += velocityIncreaser;
		verticalVelocity += velocityIncreaser;
	}*/
}
