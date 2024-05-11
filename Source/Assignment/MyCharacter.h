// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyAnimationInterface.h"
#include "Components/ArrowComponent.h"
#include "InputActionValue.h"
#include "MyCharacter.generated.h"

UCLASS()
class ASSIGNMENT_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		class UInputAction* LookAction;

	

public:
	virtual void Tick(float DeltaTime) override;
	AMyCharacter(const FObjectInitializer& ObjectInitializer);
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	//IGNORING ANY CHILD ACTORS
	FCollisionQueryParams GetIgnoreCharacterParams() const;

	//OVERRIDING JUMP
	virtual void Jump() override;
	virtual void StopJumping() override;

	class UMyCharacterMovementComponent* MyCharMovem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool Wpressed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool Spressed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool Apressed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool Dpressed;
	FVector2D PlayerMovementVector;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Arrow)
		class UArrowComponent* LeftTracerArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Arrow)
		class UArrowComponent* RightTracerArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Arrow)
		class UArrowComponent* LeftLedgeArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Arrow)
		class UArrowComponent* RightLedgeArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Arrow)
		class UArrowComponent* LeftTurnArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Arrow)
		class UArrowComponent* RightTurnArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Arrow)
		class UArrowComponent* JumpUpArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Arrow)
		class UArrowComponent* JumpDownArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Arrow)
		class UArrowComponent* PlayerUpArrow;
	
public:
	//CUSTOM JUMP BOOL
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = value)
	   bool bPressedMyChaJump;

	//STAMINA FLOAT
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = value)
		float MyStamina = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = value)
		bool IsJumping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = value)
		bool TurnBack;
	UFUNCTION(BlueprintImplementableEvent)
		void PlayerLanded();

	
protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Called when player land
	virtual void Landed(const FHitResult& Hit) override;
	


	

};
