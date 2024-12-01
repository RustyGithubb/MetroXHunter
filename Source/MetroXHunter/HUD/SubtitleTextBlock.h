/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "SubtitleTextBlock.generated.h"

/**
 * 
 */
UCLASS()
class METROXHUNTER_API USubtitleTextBlock : public UTextBlock
{
	GENERATED_BODY()
	
public:
	USubtitleTextBlock( const FObjectInitializer& ObjectInitializer );

private:
	void OnSubtitleChanged( const FText& Text );
};
