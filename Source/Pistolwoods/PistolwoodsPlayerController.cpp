// Copyright Epic Games, Inc. All Rights Reserved.

#include "PistolwoodsPlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "PistolwoodsCharacter.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "DrawDebugHelpers.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>

APistolwoodsPlayerController::APistolwoodsPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
	bAiming = false;
	thetaAmplitude = 15.0f;
}

void APistolwoodsPlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Start in Game Only Input Mode
	SetInputMode(FInputModeGameOnly());

	//Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void APistolwoodsPlayerController::PlayerTick(float DeltaTime) {
	// Call the base class
	Super::PlayerTick(DeltaTime);

	thetaAmplitude -= bAiming * 3 * DeltaTime;
	thetaAmplitude = FMath::Clamp(thetaAmplitude, 0, 15);

	// Set Mouse Position
	OnSetDestinationTriggered();
	CalculateAim();
}

void APistolwoodsPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{

		// Setup mouse input events
		// EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &APistolwoodsPlayerController::OnSetDestinationTriggered);

		// Setup Aiming action events
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &APistolwoodsPlayerController::StartAiming);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APistolwoodsPlayerController::StopAiming);

		// Setup Keypress input events
		EnhancedInputComponent->BindAction(MoveLeftKeyPress, ETriggerEvent::Triggered, this, &APistolwoodsPlayerController::MoveLeft);
		EnhancedInputComponent->BindAction(MoveRightKeyPress, ETriggerEvent::Triggered, this, &APistolwoodsPlayerController::MoveRight);
		EnhancedInputComponent->BindAction(MoveUpKeyPress, ETriggerEvent::Triggered, this, &APistolwoodsPlayerController::MoveUp);
		EnhancedInputComponent->BindAction(MoveDownKeyPress, ETriggerEvent::Triggered, this, &APistolwoodsPlayerController::MoveDown);
	}
}

void APistolwoodsPlayerController::MoveLeft()
{

	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector globalLeft = FVector(0, -1, 0);
		ControlledPawn->AddMovementInput(globalLeft, 1.0 - bAiming * 0.5, false);
	}
}

void APistolwoodsPlayerController::MoveRight()
{

	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector globalRight = FVector(0, 1, 0);
		ControlledPawn->AddMovementInput(globalRight, 1.0 - bAiming * 0.5, false);
	}
}

void APistolwoodsPlayerController::MoveUp()
{

	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector globalUp = FVector(1, 0, 0);
		ControlledPawn->AddMovementInput(globalUp, 1.0 - bAiming*0.5, false);
	}
}

void APistolwoodsPlayerController::MoveDown()
{

	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector globalDown = FVector(-1, 0, 0);
		ControlledPawn->AddMovementInput(globalDown, 1.0 - bAiming * 0.5, false);
	}
}

//void APistolwoodsPlayerController::OnInputStarted()
//{
//	StopMovement();
//}

// Triggered every frame when the input is held down
void APistolwoodsPlayerController::OnSetDestinationTriggered()
{

	// We look for the location in the world where the player has pressed the input
	FHitResult Hit;
	bool bHitSuccessful = false;
	bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);

	// If we hit a surface, cache the location
	if (bHitSuccessful)
	{
		CachedDestination = Hit.Location;
	}

	FacePointOfInterest();
}

void APistolwoodsPlayerController::SetPointOfInterest(FVector destination)
{
	CachedDestination = destination;
}

void APistolwoodsPlayerController::FacePointOfInterest()
{
	// Move towards mouse pointer or touch
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();

		aimVector = WorldDirection.GetSafeNormal();

		// Face cached point
		SetControlRotation(WorldDirection.Rotation());
	}

}

void APistolwoodsPlayerController::StartAiming() {

	bAiming = true;

	FHitResult Hit;
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();

		// Face cached point
		SetControlRotation(WorldDirection.Rotation());


		FVector TraceStart = ControlledPawn->GetActorLocation();
		// FVector TraceEnd = ControlledPawn->GetActorLocation() + WorldDirection.GetSafeNormal() * 500.0f;
		FVector TraceEnd = ControlledPawn->GetActorLocation() + aimVector * 1000.0f;

		TEnumAsByte<ECollisionChannel> TraceChannelProperty = ECC_Visibility;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, TraceChannelProperty, QueryParams);

		DrawDebugLine(GetWorld(), TraceStart, 
			Hit.bBlockingHit ? TraceStart + aimVector * Hit.Distance : TraceEnd,
			Hit.bBlockingHit ? FColor::Blue : FColor::Red, false, 0.0f, 0, 3.0f);

		if (Hit.bBlockingHit)
		{
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 0.0f, 1, FColor::Red, false, 1.0f, 1, 10.0f);
		}
	}
	
}

void APistolwoodsPlayerController::StopAiming()
{
	thetaAmplitude = 15.0f;
	bAiming = false;
	SetInputMode(FInputModeGameOnly());

	return;
}

void APistolwoodsPlayerController::CalculateAim()
{
	aimVector = aimVector.ToOrientationRotator().Add(0, thetaAmplitude * sin(UGameplayStatics::GetRealTimeSeconds(GetWorld())*PI), 0).Vector();
	return;
}
