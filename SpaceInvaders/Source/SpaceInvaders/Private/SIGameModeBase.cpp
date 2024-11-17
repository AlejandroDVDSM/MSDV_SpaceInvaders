// Fill out your copyright notice in the Description page of Project Settings.


#include "SIGameModeBase.h"

#include "InvaderSquad.h"
#include "SIPawn.h"
#include "SIPlayerController.h"
#include "Kismet/GameplayStatics.h"

ASIGameModeBase::ASIGameModeBase()
	: spawnLocation{}
	  , spawnedInvaderSquad{}

{
	DefaultPawnClass = ASIPawn::StaticClass();
	PlayerControllerClass = ASIPlayerController::StaticClass();
	InvaderSquadClass = AInvaderSquad::StaticClass();
}

// En BeginPlay ordenamos la generación de una InvaderSquad

void ASIGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	this->NewSquad.AddUObject(this, &ASIGameModeBase::OnNewSquad);
	this->PlayerZeroLifes.BindUObject(this, &ASIGameModeBase::OnPlayerZeroLifes);
	
	//Spawn a squad of invaders
	RegenerateSquad();
}

void ASIGameModeBase::RegenerateSquad()
{	
	if (InvaderSquadClass)
	{		
		if (this->spawnedInvaderSquad == nullptr)
		{
			// If no squad has been created, create one
			AInvaderSquad* squad = Cast<AInvaderSquad>(InvaderSquadClass->GetDefaultObject());
			this->spawnedInvaderSquad = Cast<AInvaderSquad>(GetWorld()->SpawnActor(InvaderSquadClass, &spawnLocation));
		} else
		{
			// If there is already a squad, get its velocity and destroy it 
			float horizontalVelocity = this->spawnedInvaderSquad->GetHorizontalVelocity();
			float verticalVelocity = this->spawnedInvaderSquad->GetVerticalVelocity();
			this->spawnedInvaderSquad->Destroy();

			// Create a new one and set its velocity based on the previous squad velocity but increased
			AInvaderSquad* squad = Cast<AInvaderSquad>(InvaderSquadClass->GetDefaultObject());
			this->spawnedInvaderSquad = Cast<AInvaderSquad>(GetWorld()->SpawnActor(InvaderSquadClass, &spawnLocation));
			spawnedInvaderSquad->horizontalVelocity = horizontalVelocity;
			spawnedInvaderSquad->verticalVelocity = verticalVelocity;
			spawnedInvaderSquad->IncrementVelocitySquad();
		}
	}
}

void ASIGameModeBase::OnNewSquad(int32 lifes)
{
	RegenerateSquad();
}

void ASIGameModeBase::EndGame() {
	if (this->spawnedInvaderSquad != nullptr)
		this->spawnedInvaderSquad->Destroy();
	
	// GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("Nuevo juego")));

	// Close level and open menu level
	UGameplayStatics::OpenLevel(this, FName("Menu"));

}

void ASIGameModeBase::OnPlayerZeroLifes() {
	EndGame();
}