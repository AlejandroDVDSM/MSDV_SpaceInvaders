// Fill out your copyright notice in the Description page of Project Settings.


#include "Invader.h"
#include "Bullet.h"
#include "InvaderMovementComponent.h"
#include "SIGameModeBase.h"

#include "NiagaraFunctionLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
AInvader::AInvader()
	: fireRate{0.0001f}
	  , bulletVelocity{3000.0f}
	  , bulletClass{ABullet::StaticClass()}
	  , positionInSquad{}
	  , timeFromLastShot{}
	  , leftSideTag{FName(AInvader::leftSideTagString)}
	  , rightSideTag{FName(AInvader::rightSideTagString)}
	  , downSideTag{FName(AInvader::downSideTagString)}
	  , bFrozen{false}
	  , bPause{false}
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	// Create Components in actor

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("BaseMeshComponent");
	RootComponent = Mesh; // We need a RootComponent to have a base transform

	// SetInvaderMesh();

	// Audio component
	AudioComponent = CreateDefaultSubobject<UAudioComponent>("Audio");
	AudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

	Movement = CreateDefaultSubobject<UInvaderMovementComponent>("InvaderMoveComponent");
	AddOwnedComponent(Movement);
	// Because UInvaderMovementComponent is only an Actor Component and not a Scene Component can't Attach To.
}

void AInvader::SetInvaderMesh(UStaticMesh* newStaticMesh, const FString path, FVector scale)
{
	const TCHAR* tpath;
	tpath = AInvader::defaultStaticMeshName; // default route
	if (!Mesh) // No Mesh component
		return;

	if (!newStaticMesh)
	{
		if (!path.IsEmpty())
			tpath = *path;
		auto MeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(tpath);
		newStaticMesh = MeshAsset.Object;
	}
	if (newStaticMesh)
	{
		Mesh->SetStaticMesh(newStaticMesh);
		// Mesh->SetRelativeScale3D(scale);
		FBoxSphereBounds meshBounds = Mesh->Bounds;
		boundOrigin = meshBounds.Origin;
		boundRadius = meshBounds.SphereRadius;
	}
}

// Called when the game starts or when spawned
void AInvader::BeginPlay()
{
	Super::BeginPlay();

	SetInvaderMesh(InvaderMeshes[FMath::RandRange(0, InvaderMeshes.Num() - 1)]);
	
	// Generate a Bullet Template of the correct class
	if (bulletClass->IsChildOf<ABullet>())
		bulletTemplate = NewObject<ABullet>(this, bulletClass->GetFName(), RF_NoFlags, bulletClass.GetDefaultObject());
	else
		bulletTemplate = NewObject<ABullet>();

	bulletTemplate->bulletType = BulletType::INVADER;
}

// Called every frame
void AInvader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bFrozen)
	{
		//Frozing the invader when is shooted down
		Movement->state = InvaderMovementType::STOP;
		return;
	}

	this->timeFromLastShot += DeltaTime;

	// Fire?
	float val = FMath::RandRange(0.0f, 1.0f);
	if (!bFrozen && val < (1.0 - FMath::Exp(-fireRate * this->timeFromLastShot)))
		Fire();
}

void AInvader::Fire()
{
	FVector spawnLocation = GetActorLocation();
	FRotator spawnRotation = GetActorRotation();
	ABullet* spawnedBullet;
	if (this->bulletTemplate)
	{
		this->bulletTemplate->velocity = bulletVelocity;
		this->bulletTemplate->dir = GetActorForwardVector();
		FActorSpawnParameters spawnParameters;
		spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		spawnParameters.Template = this->bulletTemplate;
		spawnedBullet = (ABullet*)GetWorld()->SpawnActor<ABullet>(spawnLocation, spawnRotation, spawnParameters);

		if (AudioComponent != nullptr && AudioShoot != nullptr)
		{
			AudioComponent->SetSound(AudioShoot);
			AudioComponent->Play();
		}

		this->timeFromLastShot = 0.0f;
	}
}

void AInvader::NotifyActorBeginOverlap(AActor* OtherActor)
{
	// Debug
	//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::Printf(TEXT("%s entered me"), *(OtherActor->GetName())));
	FName actorTag;
	if (bFrozen) // If it is already a zombie invader nothing happens.
		return;

	UWorld* TheWorld = GetWorld();
	if (TheWorld != nullptr)
	{
		UInvaderMovementComponent* imc = (UInvaderMovementComponent*)this->GetComponentByClass(
			UInvaderMovementComponent::StaticClass());
		bool bFreeJump = imc->state == InvaderMovementType::FREEJUMP;
		AGameModeBase* GameMode = UGameplayStatics::GetGameMode(TheWorld);
		ASIGameModeBase* MyGameMode = Cast<ASIGameModeBase>(GameMode);
		UClass* otherActorClass = OtherActor->GetClass();

		//First, bullet cases
		if (OtherActor->IsA(ABullet::StaticClass()))
		{
			ABullet* bullet = Cast<ABullet>(OtherActor);
			if (bullet->bulletType == BulletType::PLAYER)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("Invader %d killed"), this->positionInSquad));

				OtherActor->Destroy();
				MyGameMode->InvaderDestroyed.Broadcast(this->positionInSquad);
				InvaderDestroyed();
				return;
			}
			else
				return; //It's an invader bullet, so it has to be ignored
		}

		// OVerlap with other Invader is ignored
		if (OtherActor->IsA(AInvader::StaticClass()))
			return;

		// Overlap with anything in freejump (except invaders and their own bullets) is a silent Destroy.
		if (bFreeJump)
		{
			MyGameMode->InvaderDestroyed.Broadcast(this->positionInSquad);
			Destroy();
			return;
		}

		// Squad collides with limits
		if (OtherActor->ActorHasTag(leftSideTag) && !bFreeJump)
			MyGameMode->SquadOnLeftSide.ExecuteIfBound();
		else if (OtherActor->ActorHasTag(rightSideTag) && !bFreeJump)
			MyGameMode->SquadOnRightSide.ExecuteIfBound();
		else if (OtherActor->ActorHasTag(downSideTag) && !bFreeJump)
		{
			MyGameMode->SquadSuccessful.ExecuteIfBound(); // Squad wins!
		}
	}
}

void AInvader::InvaderDestroyed()
{
	UWorld* TheWorld;
	TheWorld = GetWorld(); // To get utilities as the timer.


	if (TheWorld)
	{
		bFrozen = true; // Invader can'tmove or fire while being destroyed

		UStaticMeshComponent* LocalMeshComponent = Cast<UStaticMeshComponent>(
			GetComponentByClass(UStaticMeshComponent::StaticClass()));
		// Hide Static Mesh Component
		if (LocalMeshComponent != nullptr)
		{
			LocalMeshComponent->SetVisibility(false);
		}
		//Audio
		if (AudioComponent != nullptr && AudioExplosion != nullptr)
		{
			AudioComponent->SetSound(AudioExplosion);
			AudioComponent->Play();
		}

		if (ExplosionEffect)
			UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(ExplosionEffect, Mesh, NAME_None, FVector(0.f), FRotator(0.f), EAttachLocation::Type::KeepRelativeOffset, true);
			
		
		// Wait:
		TheWorld->GetTimerManager().SetTimer(timerHandle, this, &AInvader::PostInvaderDestroyed, 2.0f, false);
	}
}

void AInvader::PostInvaderDestroyed()
{
	Destroy();
}

void AInvader::SetPositionInSquad(int32 index)
{
	this->positionInSquad = index;
}

int32 AInvader::GetPositionInSquad()
{
	return int32(this->positionInSquad);
}

float AInvader::GetBoundRadius()
{
	return this->boundRadius;
}
