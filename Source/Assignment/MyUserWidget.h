// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyCharacter.h"
#include "Components/ProgressBar.h"
#include "Blueprint/UserWidget.h"
#include "MyUserWidget.generated.h"


/**
 * 
 */


UCLASS()
class ASSIGNMENT_API UMyUserWidget : public UUserWidget
{

	GENERATED_BODY()
		
public:

	    //CREATED PROGRESSBAR OBJECT AND A TEXT BLOCK 
		class UProgressBar* MyProgressBar;
	    class UTextBlock* MyText;
		
		


protected:

	virtual void NativeConstruct() override;
	
};
