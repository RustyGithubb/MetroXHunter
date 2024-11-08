/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "HUD/SubtitleTextBlock.h"

#include "SubtitleManager.h"

USubtitleTextBlock::USubtitleTextBlock( const FObjectInitializer& ObjectInitializer )
{
	if ( FSubtitleManager* SubtitleManager = FSubtitleManager::GetSubtitleManager() )
	{
		SubtitleManager->OnSetSubtitleText().AddUObject( this, &USubtitleTextBlock::OnSubtitleChanged );
	}
}

void USubtitleTextBlock::OnSubtitleChanged( const FText& SubtitleText )
{
	SetText( SubtitleText );
}
