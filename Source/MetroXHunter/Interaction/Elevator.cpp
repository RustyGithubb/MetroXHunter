/*
 * Implemented by Corentin Paya
 */

#include "Interaction/Elevator.h"

void AElevator::Interact()
{
	SwitchCameraTarget();
	bIsElevatorMoving = true;
}

void AElevator::EndInteraction()
{
	ResetCameraTarget();
}
