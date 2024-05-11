// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyCharacter.h"
#include "My_Wall_Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyCharacterMovementComponent.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
//CREATED CUSTOM MOVEMENT MODE
enum ECustomMovementMode
{
	Custom_Move_None			UMETA(Hidden),
	Custom_Move_Climb			UMETA(DisplayName = "Climb"),
};

UCLASS()
class ASSIGNMENT_API UMyCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:


	//CLIMB LEDGE PROPERTIES
	UPROPERTY(EditDefaultsOnly) float LedgeMaxDistance = 200;
	UPROPERTY(EditDefaultsOnly) float LedgeReachHeight = 200;
	UPROPERTY(EditDefaultsOnly) float MinLedgeDepth = 150;
	UPROPERTY(EditDefaultsOnly) float LedgeMaxSurfaceAngle = 40;
	UPROPERTY(EditDefaultsOnly) float LedgeMaxAlignmentAngle = 45;


    //CLIMB LEDGE MONTAGE
	UPROPERTY(EditDefaultsOnly) UAnimMontage* ClimbLedgeMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* TransitionClimbLedgeMontage;


	UPROPERTY(EditDefaultsOnly) UAnimMontage* DoubleJump;
	

	//CLIMB PROPERTIES
	UPROPERTY(EditDefaultsOnly) float MaxClimbSpeed = 300.f;
	UPROPERTY(EditDefaultsOnly) float BrakingDeaccelerationClimbing = 100000.f;
	UPROPERTY(EditDefaultsOnly) float ClimbReachDistance = 200.f;
	UPROPERTY(EditDefaultsOnly) float WallAttractionForce = 200000.f;

	FName TransitionName = "CLimbLedge";



	//STORING IS ROOT MOTION FROM ANIMATION
	bool bHadAnimRootMotion;

	//STORING IS TRANSITION FINISHED
	bool bTransitionFinished;

	//MOVING TO LOCATION DURING TRANSITION
	TSharedPtr<FRootMotionSource_MoveToForce> TransitionRMS;

	//MONTAGE TO BE PLAYED
	UPROPERTY(Transient) UAnimMontage* TransitionQueuedMontage;

	//MONTAGE SPEED
	float TransitionQueuedMontageSpeed;

	//TRANSITION ID
	int TransitionRMS_ID;

public:
	//UPDATING CHARACTER BEFORE ANY MOVEMENT
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	//UPDATING CHARACTER AFTER ANY MOVEMENT
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

	//RUNS WHEN MOVEMENT COMPONENT INITIALIZE
	virtual void InitializeComponent() override;

	//CUSTOM CHARACTER OBJECT REFRENCE
	AMyCharacter* MyPlayer;

	//WALL NORMAL LOCATION
	FVector ClimbWallNormal;

	//HIT LOCATION FOR CLIMB
	FVector ClimbHitLocation;

	//IF PLAYER WANT TO CLIMB
	bool bWantsToClimb = false;

	//IS CHARACTER ON LEDGE
	bool OnLedge;


	//FUNCTION TO SET CUSTOM MOVEMENT MODE 
	UFUNCTION(BlueprintPure) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;

	//FUNCTION TO SET MOVEMENT MODE 
	UFUNCTION(BlueprintPure) bool IsMovementMode(EMovementMode InMovementMode) const;

	//FUCNTION RETURN CURRENT CLIMBING STATE
	UFUNCTION(BlueprintPure) bool IsClimbing() const { return IsCustomMovementMode(Custom_Move_Climb); }

	//ON CLIMB PRESSED IN BLUEPRINT
	UFUNCTION(BlueprintCallable) void ClimbPressed();

	//ON CLIMB RELEASED IN BLUEPRINT
	UFUNCTION(BlueprintCallable) void ClimbReleased();

	//TIMER HANDLE FOR INCREASING TIMER
	FTimerHandle ClimbIncreaseStaminaTimer;

	//INCREASE STAMINA FUNCTION
	void IncreaseStamina();
	
	void ForwardTracer();

	void HeightTracer(FVector WallNormal, FVector WallLocation , bool OnlyLedge);

	void GrabLedge(FVector WallNormal, FVector WallLocation , FVector HeightLocation , bool OnlyLedge);

	void LeftTracer();

	void RightTracer();

	void MoveInLedge();

	void LeftJumpTracer();

	void RightJumpTracer();

	void JumpLeftLedge();

	void JumpRightLedge();

	void StartTracingAgain();

	void TurnLeftTracer();

	void TurnRightTracer();
	
	void JumpUpTracer();

	void JumpDownTracer();

	void JumpUpLedge();

	void JumpDownLedge();

	UFUNCTION(BlueprintCallable)
	    void ExitLedge();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite , Category = value)
	    bool IsHanging;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = value)
		bool IsClimbingLedge;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   bool IsHangingPoint;


	class IMyAnimationInterface* AnimObjRef;

	UFUNCTION(BlueprintCallable)
	        void ClimbLedgeCustomEvent();

	UFUNCTION(BlueprintCallable)
		void MoveCustomEvent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   bool CanMoveLeft;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   bool CanMoveRight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   bool MovingLeft;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   bool MovingRight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   bool CanJumpLeft ;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   bool CanJumpRight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   bool CanTurnLeft;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   bool CanTurnRight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   bool CanJumpUp;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   bool CanJumpDown;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   bool CanTrace = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   float LeftJumpheight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   float RightJumpheight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   bool OnHangingLedge;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)   bool RecentlyJumped;
	
	UPROPERTY()
	     TArray<AActor*>ActorstoIgnore9;
//	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	
	
private:

	//RETURN CAPSULE RADIUS
	float ChaCapsuleRadius() const;

	//RETURN CANJUMP IN CURRENT MOVEMENT MODE
	bool CanAttemptJump() const;

	//RETURN CAPSULE HEIGHT
	float ChaCapsuleHeight() const;

	//RETURN MAX SPEED IN CURRENT MOVEMENT MODE
	float GetMaxSpeed() const;

	//RETURN BRAKINGDEACCELERATION
	float GetMaxBrakingDeacceleration() const;
	
	//SETTING CUSTOM PHYS FUNCTION
	void PhysCustom(float deltaTime, int32 iterations);

	//TRY CLIMB LEDGE
	bool TryClimbLedge();

	//TRY CLIMB 
	bool TryClimb();

	
	
	//CUSTOM PHYSCLIMB FOR CLIMBING
	void PhysClimb(float deltaTime, int32 Iterations);
	FVector GetClimbLedgeStartLocation(FHitResult FrontHit, FHitResult SurfaceHit) const;
	
};
