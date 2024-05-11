// Fill out your copyright notice in the Description page of Project Settings.


#include "My_Wall_Actor.h"

// Sets default values
AMy_Wall_Actor::AMy_Wall_Actor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMy_Wall_Actor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMy_Wall_Actor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

