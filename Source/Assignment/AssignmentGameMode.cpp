// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssignmentGameMode.h"
#include "AssignmentCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAssignmentGameMode::AAssignmentGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
