#include "MyGameMode.h"
#include "MyPawn.h"
#include "MyPlayerController.h"

AMyGameMode::AMyGameMode()
{
	DefaultPawnClass = AMyGameMode::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();
}
