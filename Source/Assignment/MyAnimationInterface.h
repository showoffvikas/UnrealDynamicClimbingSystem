// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MyAnimationInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UMyAnimationInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ASSIGNMENT_API IMyAnimationInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interact")
		void CanGrab(bool CanGrabBi);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interact")
		void ClimbLedge(bool IsClimbing);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interact")
		void CanGrabPoint(bool CanGrabPi);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interact")
		void MoveLeftAndRight(float Direction);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interact")
		void JumpLeft(bool CanJumpLeft);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interact")
		void JumpRight(bool CanJumpRight);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interact")
		void JumpUp(bool CanJumpUp);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interact")
		void JumpDown(bool CanJumpDown);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Interact")
		void TurnBack(bool CanTurnBack);
};
