// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Movement.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "EngineUtils.h"
#include "MovementCharacter.h"
#include <string>

//////////////////////////////////////////////////////////////////////////
// AMovementCharacter

AMovementCharacter::AMovementCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	//set initial movement input
	movementNumber = 0.0f;
	lastMovementNumber = 0.0f;
	movementMultiplier = 0.0f;
	startingWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void AMovementCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	MoveCharacter(movementNumber, lastMovementNumber, movementMultiplier);
	
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMovementCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("ForwardMovement", IE_Pressed, this, &AMovementCharacter::ForwardMovement);
	PlayerInputComponent->BindAction("ForwardMovement", IE_DoubleClick, this, &AMovementCharacter::FullForward);
	PlayerInputComponent->BindAction("BackwardMovement", IE_Pressed, this, &AMovementCharacter::BackwardMovement);
	PlayerInputComponent->BindAction("BackwardMovement", IE_DoubleClick, this, &AMovementCharacter::FullBackward);
	PlayerInputComponent->BindAction("StopMovement", IE_Pressed, this, &AMovementCharacter::StopMovement);

	//PlayerInputComponent->BindAxis("MoveForward", this, &AMovementCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMovementCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMovementCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMovementCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMovementCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMovementCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AMovementCharacter::OnResetVR);
}


void AMovementCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AMovementCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AMovementCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AMovementCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMovementCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
/*
void AMovementCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		//const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		GetCharacterMovement()->bOrientRotationToMovement = false;
		const FVector Direction = GetActorForwardVector();
		AddMovementInput(Direction, Value);
	}
}
*/
void AMovementCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		FRotator Rotation = GetActorRotation();
		Rotation = Rotation.Add(0.0f, 2*Value, 0.0f);
		
		//rotate actor 
		SetActorRotation(Rotation);
	}
}

void AMovementCharacter::ForwardMovement() {
	if (movementNumber < 4) {
		movementNumber++;
	}
}

void AMovementCharacter::BackwardMovement() {
	if (movementNumber > -4) {
		movementNumber--;
	}
}

void AMovementCharacter::StopMovement() {
	movementNumber = 0.0f;
}

void AMovementCharacter::FullForward() {
	movementNumber = 4.0f;
}

void AMovementCharacter::FullBackward() {
	
	movementNumber = -4.0f;
}

void AMovementCharacter::MoveCharacter(float aMovementNumber, float aLastMovementNumber, float aMovementMultiplier) {
	if (aMovementNumber > 0 && aLastMovementNumber >= 0) {
		if (aMovementMultiplier != aMovementNumber && aMovementNumber > aMovementMultiplier) {
			movementMultiplier += 0.01f;
		}
		else if (aMovementNumber != aMovementMultiplier && aMovementNumber < aMovementMultiplier) {
			movementMultiplier -= 0.01f;
		}
		GetCharacterMovement()->MaxWalkSpeed = startingWalkSpeed * movementMultiplier;
		const FVector Direction = GetActorForwardVector();
		AddMovementInput(Direction, 1.0f);
		lastMovementNumber = movementNumber;
	}
	else if (aMovementNumber > 0 && aLastMovementNumber < 0) {
		if (aMovementMultiplier > 0.0f) {
			movementMultiplier -= 0.01f;
			GetCharacterMovement()->MaxWalkSpeed = startingWalkSpeed * movementMultiplier;
			const FVector Direction = GetActorForwardVector();
			AddMovementInput(Direction, -1.0f);
		}
		else {
			lastMovementNumber = movementNumber;
		}
	}
	else if (aMovementNumber < 0.0f && aLastMovementNumber <= 0.0f) {
		if (aMovementMultiplier != (aMovementNumber * (-1)) && (aMovementNumber * (-1)) > aMovementMultiplier) {
			movementMultiplier += 0.01f;
		}
		else if (aMovementNumber != (aMovementNumber * (-1)) && (aMovementNumber * (-1)) < aMovementMultiplier) {
			movementMultiplier -= 0.01f;
		}
		GetCharacterMovement()->MaxWalkSpeed = startingWalkSpeed * movementMultiplier;
		const FVector Direction = GetActorForwardVector();
		AddMovementInput(Direction, -1.0f);
		lastMovementNumber = movementNumber;
	}
	else if (aMovementNumber < 0.0f && aLastMovementNumber > 0.0f) {
		if (aMovementMultiplier > 0.0f) {
			movementMultiplier -= 0.01f;
			GetCharacterMovement()->MaxWalkSpeed = startingWalkSpeed * movementMultiplier;
			const FVector Direction = GetActorForwardVector();
			AddMovementInput(Direction, 1.0f);
		}
		else {
			lastMovementNumber = movementNumber;
		}
	}
	else if (aMovementNumber == 0.0f) {
		if (aMovementMultiplier > aMovementNumber) {
			movementMultiplier -= 0.01f;
			GetCharacterMovement()->MaxWalkSpeed = startingWalkSpeed * movementMultiplier;
			if (aLastMovementNumber > 0) {
				const FVector Direction = GetActorForwardVector();
				AddMovementInput(Direction, 1.0f);
			}
			else if (aLastMovementNumber < 0) {
				const FVector Direction = GetActorForwardVector();
				AddMovementInput(Direction, -1.0f);
			}
		}
		else {
			lastMovementNumber = 0.0f;
		}
	}
}