#include "MyPawn.h"
#include "EnhancedInputComponent.h"
#include "MyPlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "GameFramework/FloatingPawnMovement.h"

AMyPawn::AMyPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	RootComponent = CapsuleComponent;
	CapsuleComponent->InitCapsuleSize(42.f, 96.0f);

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(CapsuleComponent);

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = true;

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));

	CapsuleComponent->SetSimulatePhysics(false);
	SkeletalMeshComponent->SetSimulatePhysics(false);

	Gravity = -980.0f;
	JumpStrength = 400.0f;
	bIsJumping = false;
	GroundZ = 0.0f;
}

void AMyPawn::BeginPlay()
{
	Super::BeginPlay();
	GroundZ = GetActorLocation().Z;

}

void AMyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Location = GetActorLocation();

	if (Location.Z > GroundZ || bIsJumping)
	{
		Velocity.Z += Gravity * DeltaTime;
		Location += Velocity * DeltaTime;

		if (Location.Z <= GroundZ)
		{
			Location.Z = GroundZ;
			Velocity.Z = 0.0f;
			bIsJumping = false;
		}

		SetActorLocation(Location);
	}

}

void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(PlayerController->MoveAction, ETriggerEvent::Triggered, this, &AMyPawn::Move);
			}

			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(PlayerController->JumpAction, ETriggerEvent::Triggered, this, &AMyPawn::Jump);
			}

			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(PlayerController->LookAction, ETriggerEvent::Triggered, this, &AMyPawn::Look);
			}
		}
	}
}

void AMyPawn::Move(const FInputActionValue& value)
{
	if (!Controller) return;

	const FVector2D MoveInput = value.Get<FVector2D>();

	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(GetActorForwardVector(), MoveInput.X);
	}

	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(GetActorRightVector(), MoveInput.Y);
	}

}

void AMyPawn::Look(const FInputActionValue& value)
{
	FVector2D LookInput = value.Get<FVector2D>();

	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

void AMyPawn::Jump(const FInputActionValue& value)
{
	if (!bIsJumping)
	{
		bIsJumping = true;
		Velocity.Z = JumpStrength;
	}
}