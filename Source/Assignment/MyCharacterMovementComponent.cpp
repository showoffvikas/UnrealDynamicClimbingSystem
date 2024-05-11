// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MyAnimationInterface.h"
#include "Kismet/GameplayStatics.h"
#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h>

// DEBUG
float MacroDuration = 8.f;
#define LINE(x1, x2, c) DrawDebugLine(GetWorld(), x1, x2, c, !MacroDuration, MacroDuration);
#define CAPSULE(x, c) DrawDebugCapsule(GetWorld(), x, ChaCapsuleHeight(), ChaCapsuleRadius(), FQuat::Identity, c, !MacroDuration, MacroDuration);



void UMyCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	
	//TRY TO CLIMB WHEN FALLING
	
	if (IsFalling() && CanTrace ) {
		ForwardTracer();
		
	}
	
	
	if (IsHanging || IsHangingPoint) {
		LeftTracer();
		RightTracer();
		MoveCustomEvent();
		LeftJumpTracer();
		RightJumpTracer();
		TurnLeftTracer();
		TurnRightTracer();
		JumpUpTracer();
		
		
	}
	if (IsHangingPoint) {
		
	}

	

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}
 
void UMyCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{

	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
	
	
}


void UMyCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
	// GETTING PLAYER OBJECT REF
	MyPlayer = Cast<AMyCharacter>(GetOwner());
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMy_Wall_Actor::StaticClass(), ActorstoIgnore9);
}


//SETTING CUSTOM MOVEMENT MODE
bool UMyCharacterMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

//SETTING DEFAULT MOVEMENT MODE
bool UMyCharacterMovementComponent::IsMovementMode(EMovementMode InMovementMode) const
{
	return InMovementMode == MovementMode;
}

//BINDED TO BP
void UMyCharacterMovementComponent::ClimbPressed()
{
  if (IsFalling() || IsClimbing()) bWantsToClimb = true;
}

//BINDED TO BP
void UMyCharacterMovementComponent::ClimbReleased()
{
	bWantsToClimb = false;
}

//TRY LEDGE CLIMB
bool UMyCharacterMovementComponent::TryClimbLedge()
{
	
	//IF NOT WALKING OR FALLING THEN RETURN
	if (!(IsMovementMode(MOVE_Walking)) && !(IsMovementMode(MOVE_Falling))) {
		return false;
	}

	//GET BASE LOCATION OF CHARACTER
	FVector BaseLoc = UpdatedComponent->GetComponentLocation()+ FVector::DownVector * ChaCapsuleHeight();

	//GET FORWARD LOCATION
	FVector FwdLoc = UpdatedComponent->GetForwardVector().GetSafeNormal2D();

	//SETTING MAXHEIGHT
	float MaxHeight = ChaCapsuleHeight() * 2 + LedgeReachHeight;


	//CHECKING SURFACE ANGLE
	float LedgeMaxSurfaceAngleCos = FMath::Cos(FMath::DegreesToRadians(LedgeMaxSurfaceAngle));

	//CHECKING ALIGNEMENT
	float LedgeAlignementAngleCos = FMath::Cos(FMath::DegreesToRadians(LedgeMaxAlignmentAngle));

	//IGNORING SELF 
	auto Params = MyPlayer->GetIgnoreCharacterParams();
	FHitResult FrontHit;

	//TRACE START LOCATION
	FVector FrontStart = BaseLoc + FVector::UpVector * (MaxStepHeight - 1);

	//TRACE END LOCATION
	FVector plg = FrontStart + MyPlayer->GetActorForwardVector() * 70;

	//LOOP FOR 2 LINE TRACE
	for (int i = 0; i < 1; i++) {
		LINE(FrontStart, plg, FColor::Red)
		if (GetWorld()->LineTraceSingleByProfile(FrontHit, FrontStart, plg, "BlockAll", Params)) {
			UE_LOG(LogTemp, Warning,TEXT("createdLine"));
			FrontStart += FVector::UpVector * (2.f * ChaCapsuleHeight() - (MaxStepHeight - 1)) / 5;
			break;
		}
	}
	
	//IF NOT HITTING ANYTHING IN FRONT RETURN
	if (!FrontHit.IsValidBlockingHit()) {
		return false;
	}

	
	if ( (FwdLoc | -FrontHit.Normal) < LedgeAlignementAngleCos) {
		return false;	
	}

	//STORING HIT RESULT FOR HEIGHT TRACE
	FHitResult SurfaceHit;
	
	//GETTING NORMAL OF PLANE
	FVector WallUp = FVector::VectorPlaneProject(FVector::UpVector, FrontHit.Normal).GetSafeNormal();

	//ANGLE BETWEEN NORMAL AND UP VECTOR
	float WallCos = FVector::UpVector | FrontHit.Normal;

	//GETTING SIN ANGLE
	float WallSin = FMath::Sqrt(1 - WallCos * WallCos);

	//TRACE START LOCATION
	FVector TraceStart = FrontHit.Location + FwdLoc + WallUp * (MaxHeight - (MaxStepHeight - 1)) / WallSin;

	if (!GetWorld()->LineTraceSingleByProfile(SurfaceHit, TraceStart, FrontHit.Location + FwdLoc, "BlockAll", Params)) {
		return false;
	}
	
	//IF NOT BLOCKING BY ANY OBJECT RETURN
	if (!SurfaceHit.IsValidBlockingHit() || (SurfaceHit.Normal | FVector::UpVector) < LedgeMaxSurfaceAngleCos) {
		return false;
	}
	
	//CALCULATE HEIGHT TO CLIMB
	float Height = (SurfaceHit.Location - BaseLoc) | FVector::UpVector;
	if (Height > MaxHeight) {
		return false;
	}
	
	//CALCULATE SURFACE COS
	float SurfaceCos = FVector::UpVector | SurfaceHit.Normal;

	//CALCULATE SURFACE SIN
	float SurfaceSin = FMath::Sqrt(1 - SurfaceCos * SurfaceCos);

	//GETTING LOCATION FOR CLIMB
	FVector TransitionTarget = GetClimbLedgeStartLocation(FrontHit, SurfaceHit);


	float UpSpeed = Velocity | FVector::UpVector ;

	//GETTING TRANSITION DISTANCE
	float TransDist = FVector::Dist(TransitionTarget, UpdatedComponent->GetComponentLocation());

	//SETTING MONTAGE SPEED
	TransitionQueuedMontageSpeed = FMath::GetMappedRangeValueClamped(FVector2D(-500, 750), FVector2D(.9f, .2f), UpSpeed);

	//RESETING TRANSITION
	TransitionRMS.Reset();

	TransitionRMS = MakeShared<FRootMotionSource_MoveToForce>();


	//OVERRIDING ROOT MOTION ACCUMULATE MODE
	TransitionRMS->AccumulateMode = ERootMotionAccumulateMode::Override;

	//GETTING DURATION
	TransitionRMS->Duration = FMath::Clamp(TransDist / 500.f, .1f, .25f);
	TransitionRMS->StartLocation = UpdatedComponent->GetComponentLocation();

	//SETTING TARGET LOCATION TO CLIMB
	TransitionRMS->TargetLocation = TransitionTarget;
	Velocity = FVector::ZeroVector;

	//CHANGING MOVEMENT MODE TO FLYING 
	SetMovementMode(MOVE_Flying);
	TransitionRMS_ID = ApplyRootMotionSource(TransitionRMS);

	
	//SETTING STAMINA OF PLAYER
	if (MyPlayer->MyStamina > 0.f)
		MyPlayer->MyStamina = MyPlayer->MyStamina - 1.2;

	//SETTING MONTAGE
	TransitionQueuedMontage = ClimbLedgeMontage;
	MyPlayer->PlayAnimMontage(TransitionClimbLedgeMontage, 1 / TransitionRMS->Duration);

	
	//GetWorld()->GetTimerManager().SetTimer(ClimbDeacreaseStaminaTimer, this, &UMyCharacterMovementComponent::DeacreaseStamina, 0.065, true, 0.f);


	return true;
	
	
}

bool UMyCharacterMovementComponent::TryClimb()
{

	

	//SURFACE HIT RESULT
	FHitResult SurfHit;

	//CAPSULE CURRENT LOCATION
	FVector CapLoc = UpdatedComponent->GetComponentLocation();

	//LINE TRACE TO CHECK FOR WALLS
	GetWorld()->LineTraceSingleByProfile(SurfHit, CapLoc, CapLoc + UpdatedComponent->GetForwardVector() * ClimbReachDistance, "BlockAll", MyPlayer->GetIgnoreCharacterParams());

	//IF NO HITS RETURNS
	if (!SurfHit.IsValidBlockingHit()) return false;

	//IF HIT SET NEW ROTATION BASED ON NORMAL 
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(-SurfHit.Normal, FVector::UpVector).ToQuat();

SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, false, SurfHit);

//SET CUSTOM MOVEMENT MODE
SetMovementMode(MOVE_Custom, Custom_Move_Climb);

//IS CHARACTER ON LEDGE
OnLedge = false;


bOrientRotationToMovement = false;

return true;
}

//INCREASE STAMINA FUNCTION
void UMyCharacterMovementComponent::IncreaseStamina()
{
	if (MyPlayer->MyStamina <= 10.f)
		MyPlayer->MyStamina = MyPlayer->MyStamina + 0.05;

}

void UMyCharacterMovementComponent::ForwardTracer()
{
	
	
	FVector StartLocation = MyPlayer->GetActorLocation();
	FHitResult ForwardHit;
	FVector FwdLoc = UpdatedComponent->GetForwardVector().GetSafeNormal2D();
	float LedgeAlignementAngleCos = FMath::Cos(FMath::DegreesToRadians(LedgeMaxAlignmentAngle));
	FVector EndLocation = MyPlayer->GetActorLocation() + FVector((UKismetMathLibrary::GetForwardVector(MyPlayer->GetActorRotation()).X) * 150, (UKismetMathLibrary::GetForwardVector(MyPlayer->GetActorRotation()).Y) * 150, UKismetMathLibrary::GetForwardVector(MyPlayer->GetActorRotation()).Z);
	if (UKismetSystemLibrary::SphereTraceSingle(GetWorld(), StartLocation, EndLocation, 20.f, ETraceTypeQuery::TraceTypeQuery1, false, ActorstoIgnore9, EDrawDebugTrace::None, ForwardHit, true, FLinearColor::Red, FLinearColor::Green, 0)) {
		if (ForwardHit.GetActor()->ActorHasTag("CanClimb")) {
			FVector WallLocation = ForwardHit.Location;
			FVector WallNormal = ForwardHit.Normal;
			if ((FwdLoc | -ForwardHit.Normal) < LedgeAlignementAngleCos) {
				return;
			}

			if (ForwardHit.GetActor()->ActorHasTag("DoubleJump")) {
				if (MyPlayer->bPressedJump) {
					MyPlayer->LaunchCharacter(FVector(0, 0, 700), true, true);
					CharacterOwner->PlayAnimMontage(DoubleJump, 1.f);
					return;
				}
				else {
					return;
				}
			}
			HeightTracer(WallNormal, WallLocation, false);
		}
		else {
			HeightTracer((StartLocation - StartLocation) / (StartLocation - StartLocation).Size(), StartLocation, true);
		}
	}
	else {
		HeightTracer((StartLocation - StartLocation) / (StartLocation - StartLocation).Size(), StartLocation, true);
	}
}

void UMyCharacterMovementComponent::HeightTracer(FVector WallNormal, FVector WallLocation, bool OnlyLedge)
{
	
	FVector StartLocation = FVector(MyPlayer->GetActorLocation().X, MyPlayer->GetActorLocation().Y, MyPlayer->GetActorLocation().Z + 350.f) + (UKismetMathLibrary::GetForwardVector(MyPlayer->GetActorRotation()) * 55.f);
	FHitResult HeightHit;
	FVector EndLocation = FVector(StartLocation.X, StartLocation.Y, StartLocation.Z - 350.f);
	if (UKismetSystemLibrary::SphereTraceSingle(GetWorld(), StartLocation, EndLocation, 20.f, ETraceTypeQuery::TraceTypeQuery1, false, ActorstoIgnore9, EDrawDebugTrace::None, HeightHit, true, FLinearColor::Red, FLinearColor::Green, 0)) {
		if (HeightHit.GetActor()->ActorHasTag("CanClimb")) {
			FVector HeightHitLocation = HeightHit.Location;
			if (!OnlyLedge && !RecentlyJumped) {
				if (((MyPlayer->GetMesh()->GetSocketLocation("PelvisSocket").Z - HeightHitLocation.Z) >= -120.f) && ((MyPlayer->GetMesh()->GetSocketLocation("PelvisSocket").Z - HeightHitLocation.Z) <= 0.f)) {
					if (!IsClimbingLedge) {
						if (HeightHit.GetActor()->ActorHasTag("ClimbPoint")) {
							IsHangingPoint = true;
							GrabLedge(WallNormal, WallLocation, HeightHitLocation, OnlyLedge);
							GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Calling"));
							
						}
						else {
							GrabLedge(WallNormal, WallLocation, HeightHitLocation, OnlyLedge);
							GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Calling Normal wall"));
							
						}

					}
				}
			}
			else {
				if ((HeightHitLocation.Z - (MyPlayer->GetMesh()->GetSocketLocation("PelvisSocket").Z ) >= 120.f) && ((MyPlayer->GetMesh()->GetSocketLocation("PelvisSocket").Z - HeightHitLocation.Z) <= 0.f)) {
					if (!IsClimbingLedge) {
						   if (HeightHit.GetActor()->ActorHasTag("ClimbPoint")) {
							IsHangingPoint = true;
							GrabLedge(WallNormal, WallLocation, HeightHitLocation, OnlyLedge);
							GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Calling"));
							 
						    }
						   else {
							   TArray<AActor*>ActorstoIgnoreForward;
							   FVector Start1 = MyPlayer->GetActorLocation() + ((UKismetMathLibrary::GetForwardVector(MyPlayer->GetActorRotation())) * -500);
							   FVector StartLocationForward = FVector(Start1.X, Start1.Y, HeightHitLocation.Z - 20);
							   FHitResult ForwardHit;
							   FVector FwdLoc = UpdatedComponent->GetForwardVector().GetSafeNormal2D();
							   float LedgeAlignementAngleCos = FMath::Cos(FMath::DegreesToRadians(LedgeMaxAlignmentAngle));
							   float LedgeMaxSurfaceAngleCos = FMath::Cos(FMath::DegreesToRadians(LedgeMaxSurfaceAngle));
							   //FVector EndLocationForward = MyPlayer->GetActorLocation() + FVector((UKismetMathLibrary::GetForwardVector(MyPlayer->GetActorRotation()).X) * 150, (UKismetMathLibrary::GetForwardVector(MyPlayer->GetActorRotation()).Y) * 150, UKismetMathLibrary::GetForwardVector(MyPlayer->GetActorRotation()).Z);
							   FVector EndLocationForward = FVector(MyPlayer->PlayerUpArrow->GetComponentLocation().X, MyPlayer->PlayerUpArrow->GetComponentLocation().Y, HeightHitLocation.Z - 20);;
							   if (UKismetSystemLibrary::SphereTraceSingle(GetWorld(), StartLocationForward, EndLocationForward, 20.f, ETraceTypeQuery::TraceTypeQuery1, false, ActorstoIgnoreForward, EDrawDebugTrace::None, ForwardHit, true, FLinearColor::Blue, FLinearColor::Yellow, 0)) {
								   if (ForwardHit.GetActor()->ActorHasTag("CanClimb")) {
									   WallLocation = ForwardHit.Location;
									   WallNormal = ForwardHit.Normal;
									   if ((FwdLoc | -HeightHit.Normal) == 0.f) {
										   //return;
									   }

									   GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Calling new wall"));
									   GrabLedge(WallNormal, WallLocation, HeightHitLocation, OnlyLedge);
								   }

							   }
						   }
					}

				}
			}
		}

	}
}

void UMyCharacterMovementComponent::GrabLedge(FVector WallNormal, FVector WallLocation , FVector HeightLocation, bool OnlyLedge)
{
	if (IsHangingPoint) {
		OnHangingLedge = true;
		IsHanging = false;
		float ChangeX = 0.f;
		float ChangeY = 0.f;
		SetMovementMode(MOVE_Flying);
		FLatentActionInfo MovementOverInfo;
		MovementOverInfo.CallbackTarget = this;
		MovementOverInfo.ExecutionFunction = "StopMovementImmediately";
		MovementOverInfo.Linkage = 0;
		MovementOverInfo.UUID = 123;
		FVector TargetLocation;
		TargetLocation = FVector(((WallNormal * FVector(ChangeX, ChangeY, 0.f)).X + WallLocation.X), ((WallNormal * FVector(ChangeX, ChangeY, 0.f)).Y + WallLocation.Y), HeightLocation.Z - 115);
		FRotator TargetRotation = FRotator(MyPlayer->GetActorRotation().Pitch, UKismetMathLibrary::Conv_VectorToRotator(WallNormal).Yaw , MyPlayer->GetActorRotation().Roll);
		//StopMovementImmediately();
		ChangeX = -25.f;
		ChangeY = -25.f;
		
		UKismetSystemLibrary::MoveComponentTo(MyPlayer->GetRootComponent(), TargetLocation, TargetRotation, false, false, 0.3, true, EMoveComponentAction::Move, MovementOverInfo);
		if (MyPlayer->GetMesh()->GetAnimInstance()->GetClass()->ImplementsInterface(UMyAnimationInterface::StaticClass()))
		{
			AnimObjRef = Cast<IMyAnimationInterface>(MyPlayer->GetMesh()->GetAnimInstance());
			AnimObjRef->Execute_CanGrabPoint(MyPlayer->GetMesh()->GetAnimInstance(), true);

		}
		
		
	}
	else {
		OnHangingLedge = OnlyLedge;
		IsHangingPoint = false;
		IsHanging = true;
		float ChangeX;
		float ChangeY;
		SetMovementMode(MOVE_Flying);
		FLatentActionInfo MovementOverInfo;
		MovementOverInfo.CallbackTarget = this;
		MovementOverInfo.ExecutionFunction = "StopMovementImmediately";
		MovementOverInfo.Linkage = 0;
		MovementOverInfo.UUID = 123;
		FVector TargetLocation;
		//FRotator TargetRotation = UKismetMathLibrary::MakeRotFromX(MyPlayer->GetActorForwardVector());
		FRotator TargetRotation = FRotator(MyPlayer->GetActorRotation().Pitch, UKismetMathLibrary::Conv_VectorToRotator(WallNormal).Yaw - 180, MyPlayer->GetActorRotation().Roll);
		//StopMovementImmediately();
		if (OnlyLedge) {
			ChangeX = 25;
			ChangeY = 25;
		}
		else {
			ChangeX = 30.f;
			ChangeY = 30.f;
		}
		
		TargetLocation = FVector(((WallNormal * FVector(ChangeX, ChangeY, 0.f)).X + WallLocation.X), ((WallNormal * FVector(ChangeX, ChangeY, 0.f)).Y + WallLocation.Y), HeightLocation.Z - 135);
		UKismetSystemLibrary::MoveComponentTo(MyPlayer->GetRootComponent(), TargetLocation, TargetRotation, false, false, 0.3, true, EMoveComponentAction::Move, MovementOverInfo);
		if (MyPlayer->GetMesh()->GetAnimInstance()->GetClass()->ImplementsInterface(UMyAnimationInterface::StaticClass()))
		{
			AnimObjRef = Cast<IMyAnimationInterface>(MyPlayer->GetMesh()->GetAnimInstance());
			AnimObjRef->Execute_CanGrab(MyPlayer->GetMesh()->GetAnimInstance(), true);

		}
	}
	
	//StopMovementImmediately();

}

void UMyCharacterMovementComponent::LeftTracer()
{
	
	FHitResult LeftTracerHit;
	if (UKismetSystemLibrary::CapsuleTraceSingle(GetWorld(), MyPlayer->LeftTracerArrow->GetComponentLocation(), MyPlayer->LeftTracerArrow->GetComponentLocation(), 20.f, 60.f, TraceTypeQuery1, false, ActorstoIgnore9, EDrawDebugTrace::None, LeftTracerHit, true, FLinearColor::Red, FLinearColor::Green, 0)) {
		if (LeftTracerHit.GetActor()->ActorHasTag("CanClimb")) {
			CanMoveLeft = true;
		}
		else {
			CanMoveLeft = false;
		}
		
	}
	else {
		CanMoveLeft = false;
	}
	
}

void UMyCharacterMovementComponent::RightTracer()
{
	FHitResult RightTracerHit;
	if (UKismetSystemLibrary::CapsuleTraceSingle(GetWorld(), MyPlayer->RightTracerArrow->GetComponentLocation(), MyPlayer->RightTracerArrow->GetComponentLocation(), 20.f, 60.f, TraceTypeQuery1, false, ActorstoIgnore9, EDrawDebugTrace::None,RightTracerHit, true, FLinearColor::Red, FLinearColor::Green, 0)) {
		if (RightTracerHit.GetActor()->ActorHasTag("CanClimb")) {
			CanMoveRight = true;
		}
		else {
			CanMoveRight = false;
		}

	}
	else {
		CanMoveRight = false;
	}

}

void UMyCharacterMovementComponent::MoveInLedge()
{
	
	if (CanMoveRight) {
		if (MyPlayer->Dpressed) {
			FVector NewLocation = UKismetMathLibrary::VInterpTo(MyPlayer->GetActorLocation(), (UKismetMathLibrary::GetRightVector(MyPlayer->GetActorRotation()) * 20) + MyPlayer->GetActorLocation(), UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), 5.f);
			MyPlayer->SetActorLocation(NewLocation, false, nullptr, ETeleportType::None);
			MovingRight = true;
			MovingLeft = false;
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Moving Right"));
		}
	}
    if (CanMoveLeft) {
		if (MyPlayer->Apressed) {
			FVector NewLocation = UKismetMathLibrary::VInterpTo(MyPlayer->GetActorLocation(), (UKismetMathLibrary::GetRightVector(MyPlayer->GetActorRotation()) * -20) + MyPlayer->GetActorLocation(), UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), 5.f);
			MyPlayer->SetActorLocation(NewLocation, false, nullptr, ETeleportType::None);
			MovingLeft = true;
			MovingRight = false;
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Moving Left"));
		}
	}
	
	if (!MyPlayer->Dpressed && !MyPlayer->Apressed) {
			MovingLeft = false;
			MovingRight = false;
	}
	
	

}

void UMyCharacterMovementComponent::LeftJumpTracer()
{
	FHitResult LeftJumpTracerHit;
	if (UKismetSystemLibrary::CapsuleTraceSingle(GetWorld(), MyPlayer->LeftLedgeArrow->GetComponentLocation(), MyPlayer->LeftLedgeArrow->GetComponentLocation(), 40.f, 200.f, TraceTypeQuery1, false, ActorstoIgnore9, EDrawDebugTrace::None, LeftJumpTracerHit, true, FLinearColor::Red, FLinearColor::Green, 0)) {
		if (LeftJumpTracerHit.GetActor()->ActorHasTag("CanClimb") || LeftJumpTracerHit.GetActor()->ActorHasTag("ClimbPoint")) {
			CanJumpLeft = true;
			LeftJumpheight = LeftJumpTracerHit.Location.Z;
		}
		else {
			CanJumpLeft = false;
		}

	}
	else {
		CanJumpLeft = false;
	}
	
}

void UMyCharacterMovementComponent::RightJumpTracer()
{
	FHitResult RightJumpTracerHit;
	if (UKismetSystemLibrary::CapsuleTraceSingle(GetWorld(), MyPlayer->RightLedgeArrow->GetComponentLocation(), MyPlayer->RightLedgeArrow->GetComponentLocation(), 40.f, 200.f, TraceTypeQuery1, false, ActorstoIgnore9, EDrawDebugTrace::None, RightJumpTracerHit, true, FLinearColor::Red, FLinearColor::Green, 0)) {
		if (RightJumpTracerHit.GetActor()->ActorHasTag("CanClimb") || RightJumpTracerHit.GetActor()->ActorHasTag("ClimbPoint")) {
			CanJumpRight = true;
			RightJumpheight = RightJumpTracerHit.Location.Z;
		}
		else {
			CanJumpRight = false;
		}

	}
	else {
		CanJumpRight = false;
	}
	
}

void UMyCharacterMovementComponent::JumpLeftLedge()
{
	if (MyPlayer->Apressed) {
		if (MyPlayer->IsJumping) {

		}
		else {
			SetMovementMode(MOVE_Flying);
			if (MyPlayer->GetMesh()->GetAnimInstance()->GetClass()->ImplementsInterface(UMyAnimationInterface::StaticClass()))
			{
				AnimObjRef = Cast<IMyAnimationInterface>(MyPlayer->GetMesh()->GetAnimInstance());
				AnimObjRef->Execute_JumpLeft(MyPlayer->GetMesh()->GetAnimInstance(), true);
				MyPlayer->IsJumping = true;
				FTimerHandle NewDelay;
				GetWorld()->GetTimerManager().SetTimer(NewDelay,this,&UMyCharacterMovementComponent::StartTracingAgain, 1, false, 1.f);
				IsHanging = false;
				MovingLeft = false;
				MovingRight = false;
				CanMoveLeft = false;
				CanMoveRight = false;
				
				

			}


		}
	}
}

void UMyCharacterMovementComponent::JumpRightLedge()
{
	if (MyPlayer->Dpressed) {
		if (MyPlayer->IsJumping) {

		}
		else {
			SetMovementMode(MOVE_Flying);
			if (MyPlayer->GetMesh()->GetAnimInstance()->GetClass()->ImplementsInterface(UMyAnimationInterface::StaticClass()))
			{
				AnimObjRef = Cast<IMyAnimationInterface>(MyPlayer->GetMesh()->GetAnimInstance());
				AnimObjRef->Execute_JumpRight(MyPlayer->GetMesh()->GetAnimInstance(), true);
				MyPlayer->IsJumping = true;
				FTimerHandle NewDelay;
				GetWorld()->GetTimerManager().SetTimer(NewDelay, this, &UMyCharacterMovementComponent::StartTracingAgain, 1, false, 1.f);
				IsHanging = false;
				MovingLeft = false;
				MovingRight = false;
				CanMoveLeft = false;
				CanMoveRight = false;
				
				
				
			}


		}
	}
}

void UMyCharacterMovementComponent::StartTracingAgain()
{
	/*
	
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("startin again"));
	//SetMovementMode(MOVE_Falling);
	if (MyPlayer->GetMesh()->GetAnimInstance()->GetClass()->ImplementsInterface(UMyAnimationInterface::StaticClass()))
	{
		AnimObjRef = Cast<IMyAnimationInterface>(MyPlayer->GetMesh()->GetAnimInstance());
		AnimObjRef->Execute_CanGrab(MyPlayer->GetMesh()->GetAnimInstance(), false);

	}
	
	*/
	
}

void UMyCharacterMovementComponent::TurnLeftTracer()
{
	StopMovementImmediately();
	FHitResult LeftTurnTracerHit;
	FVector StartLocation = MyPlayer->LeftTurnArrow->GetComponentLocation();
	FVector EndLocation = MyPlayer->LeftTurnArrow->GetComponentLocation();
	if (UKismetSystemLibrary::SphereTraceSingle(GetWorld(), StartLocation, EndLocation, 30.f,  TraceTypeQuery1, false, ActorstoIgnore9, EDrawDebugTrace::None, LeftTurnTracerHit, true, FLinearColor::Red, FLinearColor::Green, 0)) {
		if (LeftTurnTracerHit.GetActor()->ActorHasTag("CanClimb")) {
			CanTurnLeft = true;
		}
		else {
			CanTurnLeft = false;
		}
	}
	else {
		CanTurnLeft = false;
	}
}


void UMyCharacterMovementComponent::TurnRightTracer()
{
	StopMovementImmediately();
	FHitResult RightTurnTracerHit;
	FVector StartLocation = MyPlayer->RightTurnArrow->GetComponentLocation();
	FVector EndLocation = MyPlayer->RightTurnArrow->GetComponentLocation();
	if (UKismetSystemLibrary::SphereTraceSingle(GetWorld(), StartLocation, EndLocation, 30.f, TraceTypeQuery1, false, ActorstoIgnore9, EDrawDebugTrace::None, RightTurnTracerHit, true, FLinearColor::Red, FLinearColor::Green, 0)) {
		if (RightTurnTracerHit.GetActor()->ActorHasTag("CanClimb")) {
			CanTurnRight = true;
		}
		else {
			CanTurnRight = false;
		}
	}
	else {
		CanTurnRight = false;
	}
}

void UMyCharacterMovementComponent::JumpUpTracer()
{
	FHitResult JumpUpTracerHit;
	if (UKismetSystemLibrary::CapsuleTraceSingle(GetWorld(), MyPlayer->JumpUpArrow->GetComponentLocation(), MyPlayer->JumpUpArrow->GetComponentLocation(), 20.f,100.f , TraceTypeQuery1, false, ActorstoIgnore9, EDrawDebugTrace::None, JumpUpTracerHit, true, FLinearColor::Red, FLinearColor::Green, 0)) {
		if (JumpUpTracerHit.GetActor()->ActorHasTag("CanClimb")) {
			CanJumpUp = true;
		}
		else {
			CanJumpUp = false;
		}
	}
	else {
		CanJumpUp = false;
	}
}

void UMyCharacterMovementComponent::JumpDownTracer()
{
	FHitResult JumpDownTracerHit;
	if (UKismetSystemLibrary::CapsuleTraceSingle(GetWorld(), MyPlayer->JumpDownArrow->GetComponentLocation(), MyPlayer->JumpDownArrow->GetComponentLocation(), 20.f, 100.f, TraceTypeQuery1, false, ActorstoIgnore9, EDrawDebugTrace::None, JumpDownTracerHit, true, FLinearColor::Red, FLinearColor::Green, 0)) {
		if (JumpDownTracerHit.GetActor()->ActorHasTag("CanClimb")) {
			CanJumpDown = true;
		}
		else {
			CanJumpDown = false;
		}
	}
	else {
		CanJumpDown = false;
	}
}

void UMyCharacterMovementComponent::JumpUpLedge()
{
	if ( MyPlayer->Wpressed) {
		if (CanJumpUp) {
			if (!MyPlayer->IsJumping) {
				SetMovementMode(MOVE_Flying);
				if (MyPlayer->GetMesh()->GetAnimInstance()->GetClass()->ImplementsInterface(UMyAnimationInterface::StaticClass()))
				{
					AnimObjRef = Cast<IMyAnimationInterface>(MyPlayer->GetMesh()->GetAnimInstance());
					AnimObjRef->Execute_JumpUp(MyPlayer->GetMesh()->GetAnimInstance(), true);
					MyPlayer->IsJumping = true;
					IsHanging = false;
					MovingLeft = false;
					MovingRight = false;
					CanMoveLeft = false;
					CanMoveRight = false;


				}
			}
			else {

			}
		}
	}
}

void UMyCharacterMovementComponent::JumpDownLedge()
{
	if (MyPlayer->Spressed) {
		if (CanJumpDown) {
			if (MyPlayer->IsJumping) {
				SetMovementMode(MOVE_Flying);
				if (MyPlayer->GetMesh()->GetAnimInstance()->GetClass()->ImplementsInterface(UMyAnimationInterface::StaticClass()))
				{
					AnimObjRef = Cast<IMyAnimationInterface>(MyPlayer->GetMesh()->GetAnimInstance());
					AnimObjRef->Execute_JumpDown(MyPlayer->GetMesh()->GetAnimInstance(), true);
					MyPlayer->IsJumping = true;
					MyPlayer->DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));


				}
			}
		}
	}
}

void UMyCharacterMovementComponent::ExitLedge()
{
	SetMovementMode(MOVE_Walking);
	if (MyPlayer->GetMesh()->GetAnimInstance()->GetClass()->ImplementsInterface(UMyAnimationInterface::StaticClass()))
	{
		AnimObjRef = Cast<IMyAnimationInterface>(MyPlayer->GetMesh()->GetAnimInstance());
		AnimObjRef->Execute_CanGrab(MyPlayer->GetMesh()->GetAnimInstance(), false);

	}
	IsHanging = false;
}





//PHYSX CLIMB
void UMyCharacterMovementComponent::PhysClimb(float deltaTime, int32 Iterations)
{
	//AVOIDING NEAR ZERO VALUE WHILE TICKING
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

//	GetWorld()->GetTimerManager().SetTimer(ClimbDeacreaseStaminaTimer,this, &UMyCharacterMovementComponent::DeacreaseStamina, 0.065, true, 0.f);
	

	//SETTING JUST MOVED BEFORE
	bJustTeleported = false;

	Iterations++;
	//SAVING OLD LOCATION OF COMPONENT
	const FVector OldLocation = UpdatedComponent->GetComponentLocation()+FVector(0,0,100);
	
	//STORING HIT RESULTS
	FHitResult SurfHit, FloorHit;
	LINE(OldLocation, OldLocation + UpdatedComponent->GetForwardVector() * ClimbReachDistance, FColor::Red)
	LINE(OldLocation, OldLocation + FVector::DownVector * ChaCapsuleHeight() * 1.2f, FColor::Red)
	GetWorld()->LineTraceSingleByProfile(SurfHit, OldLocation, OldLocation + UpdatedComponent->GetForwardVector() * ClimbReachDistance, "BlockAll", MyPlayer->GetIgnoreCharacterParams());
	GetWorld()->LineTraceSingleByProfile(FloorHit, OldLocation, OldLocation + FVector::DownVector * ChaCapsuleHeight() * 1.2f, "BlockAll", MyPlayer->GetIgnoreCharacterParams());
	
	//IF CHARACTER CANT CLIMB MORE UP TO WALLL SET THE PLAYER TO LEDGE
	if (!SurfHit.IsValidBlockingHit())
	{
		
		if (!SurfHit.IsValidBlockingHit() && OnLedge == false) {
			FQuat NewRotation = FRotationMatrix::MakeFromX(-SurfHit.Normal).ToQuat();
			OnLedge = true;
		}
		return;
	}

	//IF FLOOR HIT TRACE HAPPENING RETURN PLAYER BACK TO GROUND
	else if (SurfHit.IsValidBlockingHit() && FloorHit.IsValidBlockingHit()) {
		SetMovementMode(MOVE_Falling);
		StartNewPhysics(deltaTime, Iterations);
		bOrientRotationToMovement = false;
		return;
	}

	//SETTING STAMINA
	if(MyPlayer->MyStamina>=0)
	   MyPlayer->MyStamina = MyPlayer->MyStamina - 0.02;
	
	// TRANSFORM ACCELERATION
	Acceleration.Z = 0.f;
	Acceleration = Acceleration.RotateAngleAxis(90.f, -UpdatedComponent->GetRightVector());

	// APPLY ACCELERATION
	CalcVelocity(deltaTime, 50, false, GetMaxBrakingDeceleration());
	Velocity = FVector::VectorPlaneProject(Velocity, SurfHit.Normal);

	// COMPUT MOVE PARAMERTERS
	const FVector Delta = deltaTime * Velocity; // dx = v * dt
	if (!Delta.IsNearlyZero())
	{
		//MOVING COMPONENT
		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);
		FVector WallAttractionDelta = -SurfHit.Normal * WallAttractionForce * deltaTime;
		SafeMoveUpdatedComponent(WallAttractionDelta, UpdatedComponent->GetComponentQuat(), true, Hit);

	}
	//SET VELOCITTY AFTER MOVEMENT
	Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime; // v = dx / dt
}

//SETTING CLIMB LEDGE START LOCAION
FVector UMyCharacterMovementComponent::GetClimbLedgeStartLocation(FHitResult FrontHit, FHitResult SurfaceHit) const
{
	//CALCUATION HIT ANGLE BETWEEN NORMAL
	float CosWallStipnessAngle = FrontHit.Normal | FVector::UpVector;

	//SETTING DISTANCE TO CLIMB
	float DownDistance;
	DownDistance = MyPlayer->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.f;

	//GETTING PERPENDICULAR VECTOR 
	FVector EdgeTangent = FVector::CrossProduct(SurfaceHit.Normal, FrontHit.Normal).GetSafeNormal();

	//GETTIG HIT LOCATION
	FVector ClimbLedgeStart = SurfaceHit.Location;
	ClimbLedgeStart += FrontHit.Normal.GetSafeNormal2D() * (2.f + ChaCapsuleRadius());
	ClimbLedgeStart += UpdatedComponent->GetForwardVector().GetSafeNormal2D().ProjectOnTo(EdgeTangent) * ChaCapsuleRadius() * .3f;
	ClimbLedgeStart += FVector::UpVector * (ChaCapsuleHeight());
	ClimbLedgeStart += FVector::DownVector * DownDistance;
	ClimbLedgeStart += FrontHit.Normal.GetSafeNormal2D() * CosWallStipnessAngle * DownDistance;
	return ClimbLedgeStart;

}

void UMyCharacterMovementComponent::ClimbLedgeCustomEvent()
{
	if (!IsClimbingLedge) {
		SetMovementMode(MOVE_Flying);
		if (MyPlayer->GetMesh()->GetAnimInstance()->GetClass()->ImplementsInterface(UMyAnimationInterface::StaticClass()))
		{
			AnimObjRef = Cast<IMyAnimationInterface>(MyPlayer->GetMesh()->GetAnimInstance());
			AnimObjRef->Execute_ClimbLedge(MyPlayer->GetMesh()->GetAnimInstance(), true);
		}
		IsClimbingLedge = true;
		IsHanging = false;
	}
}

void UMyCharacterMovementComponent::MoveCustomEvent()
{
	if (IsHanging) {
		if (MyPlayer->GetMesh()->GetAnimInstance()->GetClass()->ImplementsInterface(UMyAnimationInterface::StaticClass()))
		{
			AnimObjRef = Cast<IMyAnimationInterface>(MyPlayer->GetMesh()->GetAnimInstance());
			AnimObjRef->Execute_MoveLeftAndRight(MyPlayer->GetMesh()->GetAnimInstance(), 0.f);
		}
		MoveInLedge();
	}
	else {
		MovingLeft = false;
		MovingRight = false;
	}
}

//FUNCTION TO GET CAPSULE RADIUS
float UMyCharacterMovementComponent::ChaCapsuleRadius() const
{
	
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

//FUNCTION TO CHECK CAN JUMP OR NOT
bool UMyCharacterMovementComponent::CanAttemptJump() const
{
	return Super::CanAttemptJump() || IsClimbing();
}

//FUNCTION TO GET CAPSULE HALF HEIGHT
float UMyCharacterMovementComponent::ChaCapsuleHeight() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

//SETTING MAX SPEED BASED ON MOVEMENTMODE
float UMyCharacterMovementComponent::GetMaxSpeed() const
{
	if (MovementMode != MOVE_Custom) {
		return Super::MaxWalkSpeed;
	}
	switch (CustomMovementMode) {
	
	case Custom_Move_Climb:
		return MaxClimbSpeed;
	default:
		return -1;
	}

	
}

//SETTING BRAKING DEACCELERATION BASED ON MOVEMENETMODE
float UMyCharacterMovementComponent::GetMaxBrakingDeacceleration() const
{
	if (MovementMode != MOVE_Custom) return Super::GetMaxBrakingDeceleration();
	switch (CustomMovementMode) {
	
	case Custom_Move_Climb:
		return BrakingDeaccelerationClimbing;
	default:
		return -1;


	}
}

//SETTING CUSTOM PHYS BASED ON MOVEMENT MODE
void UMyCharacterMovementComponent::PhysCustom(float deltaTime, int32 iterations)
{
	Super::PhysCustom(deltaTime, iterations);
	switch (CustomMovementMode) {
	
	case Custom_Move_Climb:
		return PhysClimb(deltaTime, iterations);
	}
	
}
