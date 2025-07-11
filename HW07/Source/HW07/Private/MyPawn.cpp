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
	SpringArmComp->bUsePawnControlRotation = false;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));

	CapsuleComponent->SetSimulatePhysics(false);
	SkeletalMeshComponent->SetSimulatePhysics(false);
	bUseControllerRotationYaw = false;

	Gravity = -980.0f;
	JumpStrength = 400.0f;
	bIsJumping = false;
	GroundZ = 0.0f;

	NormalSpeed = 600;
	SprintSpeedMultiplier = 1.7;
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;
	MovementComponent->MaxSpeed = NormalSpeed;
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

			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(PlayerController->SprintAction, ETriggerEvent::Started, this, &AMyPawn::StartSprint);
				EnhancedInput->BindAction(PlayerController->SprintAction, ETriggerEvent::Completed, this, &AMyPawn::StopSprint);
			}
		}
	}
}

void AMyPawn::Move(const FInputActionValue& value)
{
	const FVector2D MoveInput = value.Get<FVector2D>();
	if (MoveInput.IsNearlyZero()) return;

	const FVector ForwardDirection = GetActorForwardVector();
	const FVector RightDirection = GetActorRightVector();

	FVector WorldMoveDirction = (ForwardDirection * MoveInput.X) + (RightDirection * MoveInput .Y);

	AddActorWorldOffset(WorldMoveDirction * MovementComponent->MaxSpeed * GetWorld()->GetDeltaSeconds(), true);
}

void AMyPawn::Look(const FInputActionValue& value)
{
	const FVector2D LookInput = value.Get<FVector2D>();
	if (LookInput.IsNearlyZero()) return;

	const float YawSensitivity = 2.0f;
	const float PitchSensitivity = -2.0f;

	FRotator CurrentRotation = GetActorRotation();
	CurrentRotation.Yaw += LookInput.X * YawSensitivity;
	SetActorRotation(CurrentRotation);

	FRotator SpringArmRot = SpringArmComp->GetRelativeRotation();
	SpringArmRot.Pitch = FMath::Clamp(SpringArmRot.Pitch + LookInput.Y * PitchSensitivity, -80.0f, 80.0f);
	SpringArmComp->SetRelativeRotation(SpringArmRot);
}

void AMyPawn::Jump(const FInputActionValue& value)
{
	if (!bIsJumping)
	{
		bIsJumping = true;
		Velocity.Z = JumpStrength;
	}
}

void AMyPawn::StartSprint(const FInputActionValue& value)
{
	if (MovementComponent)
	{
		MovementComponent->MaxSpeed = SprintSpeed;
	}
}

void AMyPawn::StopSprint(const FInputActionValue& value)
{
	if (MovementComponent)
	{
		MovementComponent->MaxSpeed = NormalSpeed;
	}
}