	// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameMode.h"
#include "MyCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMyGameMode::AMyGameMode()
{
	// SETTING DEFAULT PAWN TO OUR CUSTOM CHARACTER
	DefaultPawnClass = AMyCharacter::StaticClass();
	
}

