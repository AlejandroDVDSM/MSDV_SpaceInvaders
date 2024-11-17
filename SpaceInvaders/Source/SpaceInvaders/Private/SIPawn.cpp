// Fill out your copyright notice in the Description page of Project Settings.


#include "SIPawn.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Bullet.h"
#include "Invader.h"
#include "SIGameModeBase.h"
#include "NiagaraFunctionLibrary.h"
#include "SIGameInstance.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASIPawn::ASIPawn()
	: pointsPerInvader{100},
	  pointsPerSquad{1000},
	  playerLifes{3},
	  velocity{1000},
	  bulletVelocity{3000},

	  AudioShoot{}, //nullptr if(AudioShoot)
	  AudioExplosion{},
	  bFrozen{false},
	  bPause{false},
	  MyGameMode{},
	  playerPoints{0}
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetStaticMesh(); // Default mesh (SetStaticMesh with no arguments)
	
	// Audio component
	AudioComponent = CreateDefaultSubobject<UAudioComponent>("Audio");
	AudioComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
}

// Set a static mesh.
void ASIPawn::SetStaticMesh(UStaticMesh* staticMesh, FString path, FVector scale) {
	UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
	const TCHAR* tpath;
	tpath = ASIPawn::defaultStaticMeshPath; // default route
	if (!Mesh) // No Mesh component
		return;

	if (!staticMesh) {
		if (!path.IsEmpty())
			tpath = *path;
		auto MeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(tpath);
		staticMesh = MeshAsset.Object;
	}
	if (staticMesh) {
		Mesh->SetStaticMesh(staticMesh);

		Mesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	}
}

// Called when the game starts or when spawned
void ASIPawn::BeginPlay()
{
	Super::BeginPlay();

	// Generate a Bullet Template of the correct class
	if (bulletClass->IsChildOf<ABullet>())
		bulletTemplate = NewObject<ABullet>(this, bulletClass);
	else
		bulletTemplate = NewObject<ABullet>(this);

	bulletTemplate->bulletType = BulletType::PLAYER;

	UWorld* TheWorld = GetWorld();
	if (TheWorld != nullptr)
	{
		AGameModeBase* GameMode = UGameplayStatics::GetGameMode(TheWorld);
		MyGameMode = Cast<ASIGameModeBase>(GameMode);
		if (MyGameMode)
		{
			MyGameMode->InvaderDestroyed.AddUObject(this, &ASIPawn::InvaderDestroyed);
			MyGameMode->SquadSuccessful.BindUObject(this, &ASIPawn::SquadSuccessful);
			MyGameMode->NewSquad.AddUObject(this, &ASIPawn::SquadDissolved);
		}
	}
}

// Called every frame
void ASIPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ASIPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add mapping context
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		PlayerController->GetLocalPlayer());

	Subsystem->ClearAllMappings();
	Subsystem->AddMappingContext(InputMapping, 0);

	// Bind actions
	UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	enhancedInputComponent->BindAction(inputToMove, ETriggerEvent::Triggered, this, &ASIPawn::OnEnhancedMove);
	enhancedInputComponent->BindAction(inputToFire, ETriggerEvent::Triggered, this, &ASIPawn::OnEnhancedFire);
	enhancedInputComponent->BindAction(inputToPause, ETriggerEvent::Triggered, this, &ASIPawn::OnEnhancedPause);

	// PlayerInputComponent->BindAxis(TEXT("SIRight"), this, &ASIPawn::OnMove);
	// PlayerInputComponent->BindAction(TEXT("SIFire"), IE_Pressed, this, &ASIPawn::OnFire);
	// PlayerInputComponent->BindAction(TEXT("SIPause"), IE_Pressed, this, &ASIPawn::OnPause);
}

void ASIPawn::OnEnhancedMove(const FInputActionValue& Value)
{
	if (bFrozen)
		return;

	float deltaTime = GetWorld()->GetDeltaSeconds(); // Tiempo desde la ultima ejecucion del bucle del juego

	float delta = velocity * Value.Get<float>() * deltaTime;
	FVector dir = FVector(0.0f, 1.0f, 0.0f);

	AddMovementInput(dir, delta);
}

void ASIPawn::OnEnhancedFire()
{
	if (bFrozen)
		return;

	FVector spawnLocation = GetActorLocation();
	FRotator spawnRotation = GetActorRotation();
	ABullet* spawnedBullet;
	bulletTemplate->velocity = bulletVelocity;
	bulletTemplate->dir = GetActorForwardVector();
	FActorSpawnParameters spawnParameters;
	spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	spawnParameters.Template = bulletTemplate;
	spawnedBullet = Cast<ABullet>(GetWorld()->SpawnActor(bulletClass, &spawnLocation, &spawnRotation, spawnParameters));

	if (AudioComponent != nullptr && AudioShoot != nullptr)
	{
		AudioComponent->SetSound(AudioShoot);
	}

	AudioComponent->Play();
}

void ASIPawn::OnEnhancedPause()
{
	bPause = !bPause;
}

int64 ASIPawn::GetPoints()
{
	return this->playerPoints;
}


int32 ASIPawn::GetLifes()
{
	return this->playerLifes;
}

void ASIPawn::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (!bFrozen)
	{
		// Collision with an enemy bullet
		if (OtherActor->IsA(ABullet::StaticClass()))
		{
			// ABullet::StaticClass() obtengo un puntero a la UCLASS en memoria de Abullet
			ABullet* bullet = Cast<ABullet>(OtherActor);
			if (bullet->bulletType == BulletType::INVADER)
			{
				OtherActor->Destroy();
				DestroyPlayer();
			}
		}
		// Collision with an invader
		if (OtherActor->IsA(AInvader::StaticClass()))
		{
			OtherActor->Destroy();
			DestroyPlayer();
		}
	}
}

void ASIPawn::DestroyPlayer()
{
	UWorld* TheWorld;
	TheWorld = GetWorld();

	if (TheWorld)
	{
		bFrozen = true; // Pawn can't move or fire while being destroyed
		--this->playerLifes;
		UStaticMeshComponent* LocalMeshComponent = Cast<UStaticMeshComponent>(
			GetComponentByClass(UStaticMeshComponent::StaticClass()));
		// Hide Static Mesh Component
		if (LocalMeshComponent != nullptr)
		{
			// GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, FString::Printf(TEXT("visibility")));
			LocalMeshComponent->SetVisibility(false);
		}
		//Audio
		if (AudioComponent != nullptr && AudioExplosion != nullptr)
		{
			AudioComponent->SetSound(AudioExplosion);
			AudioComponent->Play();
		}

		if (ExplosionEffect)
		{
			UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
			UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(ExplosionEffect, Mesh, NAME_None, FVector(0.f), FRotator(0.f), EAttachLocation::Type::KeepRelativeOffset, true);
		}
		
		// Wait:
		TheWorld->GetTimerManager().SetTimer(timerHandle, this, &ASIPawn::PostPlayerDestroyed, 3.0f, false);
	}
}

void ASIPawn::PostPlayerDestroyed()
{
	// End game
	if (this->playerLifes == 0)
	{
		if (MyGameMode)
		{
			// Checks if the record has been beated
			USIGameInstance* GameInstance = Cast<USIGameInstance>(GetGameInstance());
			if (playerPoints > GameInstance->GetRecord())
				GameInstance->UpdateRecord(playerPoints);
			
			MyGameMode->PlayerZeroLifes.ExecuteIfBound();
		}
		return;
	}

	// Regenerate and continue
	UStaticMeshComponent* LocalMeshComponent = Cast<UStaticMeshComponent>(
		GetComponentByClass(UStaticMeshComponent::StaticClass()));
	// Show Static Mesh Component
	if (LocalMeshComponent != nullptr)
	{
		LocalMeshComponent->SetVisibility(true);
	}
	// Unfrozing
	bFrozen = false;
}

// Delegate responses:
void ASIPawn::InvaderDestroyed(int32 id)
{
	this->playerPoints += this->pointsPerInvader;
}


void ASIPawn::SquadSuccessful()
{
	DestroyPlayer();
	if (MyGameMode)
		MyGameMode->NewSquad.Broadcast(this->playerLifes);
}

void ASIPawn::SquadDissolved(int32 val)
{
	this->playerPoints += this->pointsPerSquad;
}
