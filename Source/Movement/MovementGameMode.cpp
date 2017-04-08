// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Movement.h"
#include "MovementGameMode.h"
#include "MovementCharacter.h"

AMovementGameMode::AMovementGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
