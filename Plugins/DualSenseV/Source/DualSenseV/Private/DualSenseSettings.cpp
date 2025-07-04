#include "DualSenseSettings.h"

FName UDualSenseSettings::GetCategoryName() const
{
	return TEXT( "Plugins" );
}

const UDualSenseSettings* UDualSenseSettings::Get()
{
	return GetDefault<UDualSenseSettings>();
}
