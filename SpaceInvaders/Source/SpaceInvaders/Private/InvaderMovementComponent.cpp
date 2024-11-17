// Fill out your copyright notice in the Description page of Project Settings.


#include "InvaderMovementComponent.h"
#include "Invader.h"
#include "SIGameModeBase.h"
#include "SIPawn.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UInvaderMovementComponent::UInvaderMovementComponent()
	: horizontalVelocity{1000.0f}
	  , verticalVelocity{1000.0f}
	  , state{InvaderMovementType::STOP}
	  , descendingStep{100.0f}
	  , numberOfTargetPoints{5}
	  , freeJumpRadius{300.0f}
	  , freeJumpVelocity{1000.0f}
	  , deltaAlphaInterpolation{1.0f / 30.0f}
	  , previousState{InvaderMovementType::STOP}
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UInvaderMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* TheWorld;

	TheWorld = GetWorld();
	if (TheWorld)
	{
		AGameModeBase* GameMode = UGameplayStatics::GetGameMode(TheWorld);
		MyGameMode = Cast<ASIGameModeBase>(GameMode);
	}

	finalAngle = FMath::RandRange(-30.0f, 30.0f);
}

// Generate a sequence of geometric transformation to perform a circular trajectory
void UInvaderMovementComponent::GenerateTargetPoints()
{
	AActor* Parent = GetOwner();
	FTransform initialTransform;
	FVector initialLocation;
	FVector initialScale;

	FQuat initialQuaternion;
	FVector forward;

	if (!Parent)
	{
		numberOfTargetPoints = 0;
		return;
	}

	initialTransform = Parent->GetActorTransform();
	initialLocation = initialTransform.GetLocation();
	initialScale = initialTransform.GetScale3D();
	initialQuaternion = initialTransform.GetRotation();
	forward = Parent->GetActorForwardVector();


	// The first stage movement is a circle
	// Calculate center of the circle from actor location
	float radio = freeJumpRadius;
	FVector center = initialLocation;

	center.X += radio;

	if (numberOfTargetPoints > 0)
	{
		float theta = 0.0f; // Ángulo a avanzar en cada punto de referencia.
		float deltaTheta = 2 * PI / numberOfTargetPoints;

		//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Blue, FString::Printf(TEXT("X is %g Y is %g"), initialLocation.X, initialLocation.Y));

		FVector newLocation = initialLocation;
		FRotator rotation;
		FTransform newTransform = initialTransform;
		for (int32 i = 0; i < numberOfTargetPoints; i++)
		{
			float pc = FMath::Cos(theta);
			float ps = FMath::Sin(theta);

			newLocation.X = center.X - radio * FMath::Cos(theta);
			newLocation.Y = center.Y + radio * FMath::Sin(theta);
			newTransform.SetLocation(newLocation);

			// Change the rotation of the actor to follow the tangent of the circle
			if (i != (numberOfTargetPoints - 1))
			{
				// FRotator requires angles in degrees!
				// Rotation is in Yaw. The following angle makes the invader to follow the tangent
				rotation = FRotator(0.0f, -(theta * 180.0f / PI) - 90, 0.0f);
				FQuat newQuaternion = rotation.Quaternion() * initialQuaternion;
				newTransform.SetRotation(newQuaternion);
			}
			else // Last point set the invader with the initial transformation
				newTransform.SetRotation(initialQuaternion); //Last transformation 
			targetPoints.Add(newTransform);
			theta += deltaTheta;
		}
	}
}

FTransform UInvaderMovementComponent::InterpolateWithTargetPoints(FTransform origin, float fraction)
{
	FVector originLocation = origin.GetLocation();
	FQuat originRotation = origin.GetRotation();
	FVector targetLocation;
	FQuat targetRotation;
	if (currentTargetPoint >= 0 && currentTargetPoint < numberOfTargetPoints)
	{
		targetLocation = targetPoints[currentTargetPoint].GetLocation();
		targetRotation = targetPoints[currentTargetPoint].GetRotation();
	}
	else
		return origin;

	// Location interporlation for vectors
	FVector newLocation = UKismetMathLibrary::VLerp(originLocation, targetLocation, fraction);
	// Spherical interpolation for quaterions
	FQuat newRotation = FQuat::Slerp(originRotation, targetRotation, fraction);

	FTransform newTransform = origin;
	newTransform.SetLocation(newLocation);
	newTransform.SetRotation(newRotation);
	return newTransform;
}

void UInvaderMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Parent = GetOwner(); //Parent is the actor who owns this component.

	if (!Parent)
		return;

	float deltaHorizontal = horizontalVelocity * DeltaTime;

	// Increment in horizontal and vertical dimensions given DeltaTime and parameterized velocities
	float deltaVertical = verticalVelocity * DeltaTime;

	// GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::Printf(TEXT("HV is %g - VV is %g - DH is %g - DV is %g"), horizontalVelocity, verticalVelocity, deltaHorizontal, deltaVertical));

	float deltaX = 0.0f; // These deltas determine the change of the actor position
	float deltaY = 0.0f;

	// deltaX and deltaY are calculated differently for each movement type
	// The movement type is in the public variable state
	// previousState is updated
	switch (state)
	{
	case InvaderMovementType::STOP:
		deltaX = 0.0f; //No variation if STOP
		deltaY = 0.0f;
		previousState = InvaderMovementType::STOP;
		break;

	case InvaderMovementType::RIGHT:

		deltaX = 0.0f;
		deltaY = deltaHorizontal;

		previousState = InvaderMovementType::RIGHT;
		break;

	case InvaderMovementType::LEFT:

		deltaX = 0.0f;
		deltaY = -deltaHorizontal;
		previousState = InvaderMovementType::LEFT;
		break;

	// Down movement: this is an automatic movement that has to finish automatically
	// It is based on an internal variable, descendinfProgress, that is updated.

	case InvaderMovementType::DOWN:
		if (previousState != InvaderMovementType::DOWN)
			descendingProgress = 0.0f; // This means  that the down phase is starting
		if (descendingProgress > descendingStep)
		{
			deltaVertical = 0.0f; // This means that the down phase stops
			MyGameMode->SquadFinishesDown.ExecuteIfBound();
		}

		deltaX = -deltaVertical;
		deltaY = 0.0f;
		descendingProgress += deltaVertical;
		previousState = InvaderMovementType::DOWN;
		break;

	// Free jump movement: this is an automatic complex movement.

	case InvaderMovementType::FREEJUMP:


		deltaX = 0.0f; // This movement is not based on deltaX, deltaY, but in general transformations
		deltaY = 0.0f;

		if (previousState != InvaderMovementType::FREEJUMP)
		{
			// First time we enter in FREEJUMP
			GenerateTargetPoints();
			currentTargetPoint = 0;
			if (numberOfTargetPoints > 0)
			{
				originTransform = Parent->GetActorTransform();
				// First originTransform for interpolation is actor transform
				alphaInterpolation = 0.0f;
			}

			previousState = InvaderMovementType::FREEJUMP;
		}

		// Now the movement is programatically defined.
		// There are two stages:
		// First stage: an automatic movement defined by a sequence of target transforms
		// currentTargetPoint is the index of the current transform
		if (currentTargetPoint < numberOfTargetPoints)
		{
			FTransform newtransform = InterpolateWithTargetPoints(originTransform, alphaInterpolation);
			// New transform calculated by interpolation between current and currentTargetPoint.
			// The actor receive the new transform					
			Parent->SetActorTransform(newtransform);

			alphaInterpolation += deltaAlphaInterpolation;
			if (alphaInterpolation > 1.0f)
			{
				// target has been reached with interpolation
				++currentTargetPoint;
				alphaInterpolation = 0.0f; // To start a new segment of linear interpolation


				if (currentTargetPoint < numberOfTargetPoints) // new originTransform is previous target
					originTransform = this->targetPoints[currentTargetPoint - 1];
				// If this was the last target we get the player position in the second stage of the free jump
				else
				{
					APawn* playerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
					if (playerPawn)
					{
						FVector playerLocation = playerPawn->GetActorLocation();
						FVector invaderLocation = Parent->GetActorLocation();

						// Calculate the direction from the invader to the player
						FVector target = playerLocation - invaderLocation;
						target.Z = 0; // Ignore the Z axis to only rotate in the horizontal plane
						
						FRotator TargetRotation = target.Rotation(); 
						Parent->SetActorRotation(TargetRotation);
					
					}
				}
			}
		}

		// Second stage: the actor is simply moved in the forward direction 

		else
		{
			FVector parentLocation = Parent->GetActorLocation();
			FVector forward = Parent->GetActorForwardVector();
			// FVector right = Parent->GetActorRightVector();
			parentLocation += freeJumpVelocity * DeltaTime * forward;

			Parent->SetActorLocation(parentLocation);
		}
	}

	// Apply calculated deltaX deltaY for those movements based on them
	if (Parent && state != InvaderMovementType::FREEJUMP)
	{
		FVector parentLocation = Parent->GetActorLocation();
		parentLocation.X += deltaX;
		parentLocation.Y += deltaY;
		Parent->SetActorLocation(parentLocation);
	}
}
