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

APistolwoodsPlayerController::APistolwoodsPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
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

void APistolwoodsPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{

		// Setup mouse input events
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &APistolwoodsPlayerController::OnSetDestinationTriggered);


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
		ControlledPawn->AddMovementInput(globalLeft, 1.0, false);
	}
}

void APistolwoodsPlayerController::MoveRight()
{

	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector globalRight = FVector(0, 1, 0);
		ControlledPawn->AddMovementInput(globalRight, 1.0, false);
	}
}

void APistolwoodsPlayerController::MoveUp()
{

	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector globalUp = FVector(1, 0, 0);
		ControlledPawn->AddMovementInput(globalUp, 1.0, false);
	}
}

void APistolwoodsPlayerController::MoveDown()
{

	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector globalDown = FVector(-1, 0, 0);
		ControlledPawn->AddMovementInput(globalDown, 1.0, false);
	}
}

void APistolwoodsPlayerController::OnInputStarted()
{
	StopMovement();
}

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

		// Face cached point
		SetControlRotation(WorldDirection.Rotation());
	}

}